#pragma once

#include <atomic>
#include <coroutine>

#include "axpch.hpp"

#include <concurrentqueue.hpp>

#include "RingBuffer.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    inline static constexpr u32 BufferCapacity = 64;
    using ThreadAffinity = u8;
    enum class JobPriority { Low = 0, Medium, High };
    template <typename T>
    using JobBufferPtr = std::unique_ptr<RingBuffer<T, BufferCapacity>>;

    constexpr ThreadAffinity InvalidThreadIndex = std::numeric_limits<ThreadAffinity>::max();

    // Forward declarations
    struct Job;
    template <typename T>
    struct JobCoroutine;
    template <typename T>
    struct JobPromise;
    template <typename T>
    struct JobPromiseBase;
    struct JobFunction;
    struct JobCounterAwaiter;
    struct JobCounterAwaitable;

    using JobPtr = std::unique_ptr<Job>;

    struct Job {
        JobPriority m_Priority{JobPriority::Medium};
        ThreadAffinity m_ThreadIndex{InvalidThreadIndex};
        bool m_IsFunction{true};
        JobCounterAwaitable* m_WaitingOn{nullptr};

        Job() = default;
        Job(JobPriority priority, ThreadAffinity threadIndex, bool isFunction)
            : m_Priority(priority),
              m_ThreadIndex(threadIndex),
              m_IsFunction(isFunction) {}

        virtual void Resume() = 0;
        virtual void Destroy() = 0;
    };

    struct JobFunction : public Job {
        std::function<void()> m_Function;

        JobFunction() = default;
        JobFunction(std::function<void()> func,
                    JobPriority priority = JobPriority::Medium,
                    ThreadAffinity threadIndex = InvalidThreadIndex)
            : m_Function(func),
              Job(priority, threadIndex, true) {}

        void Resume() override {
            m_Function();
        }

        void Destroy() override {}
    };

    template <typename T>
    struct JobCoroutine {
        using promise_type = JobPromise<T>;

        std::coroutine_handle<JobPromise<T>> m_Handle;
        bool m_Scheduled{false};

        JobCoroutine() = default;
        explicit JobCoroutine(std::coroutine_handle<JobPromise<T>> h)
            : m_Handle(h) {}

        JobCoroutine(JobCoroutine&& other) noexcept
            : m_Handle(std::exchange(other.m_Handle, {})) {}
        JobCoroutine& operator=(JobCoroutine&& other) noexcept {
            if (this != &other) {
                // Destroy our current handle if we own one
                if (m_Handle)
                    m_Handle.destroy();
                m_Handle = std::exchange(other.m_Handle, {});
            }
            return *this;
        }
        JobCoroutine(const JobCoroutine&) = delete;
        ~JobCoroutine() {
            if (m_Handle && !m_Scheduled)
                m_Handle.destroy();
        }

        std::coroutine_handle<JobPromise<T>> Release() {
            return std::exchange(m_Handle, {});
        }

        Expected<T> Get() noexcept {
            if constexpr (!std::is_void_v<std::decay_t<T>>) {
                if (m_Handle && m_Handle.done())
                    return m_Handle.promise().Get();
                else
                    return Expected<T>::FromException(
                        "Trying to get a value out of a non-existing coroutine or a non-finished one");
            }
            return Expected<T>::FromException("Trying to get a value out of a void coroutine");
        }
    };

    // This is the base class of all coroutine promises. We need it to distinguish between return type
    // of void or any value. Coroutines must have different interfaces for these two cases, but we
    // need a unified interfaces to handle all cases.
    template <typename T>
    struct JobPromiseBase : public Job {
        std::coroutine_handle<> m_Handle;

        JobPromiseBase(std::coroutine_handle<> handle)
            : m_Handle(handle),
              Job(JobPriority::Medium, InvalidThreadIndex, false) {}

        std::suspend_always initial_suspend() noexcept {
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {
            std::terminate();
        }

        virtual void Resume() override {
            if (m_Handle && !m_Handle.done())
                m_Handle.resume();
        }
        virtual void Destroy() override {
            m_Handle.destroy();
        }
    };

    template <typename T>
    struct JobPromise : public JobPromiseBase<T> {
        JobPromise() noexcept
            : JobPromiseBase<T>(std::coroutine_handle<JobPromise<T>>::from_promise(*this)) {}

        JobCoroutine<T> get_return_object() {
            return JobCoroutine<T>(std::coroutine_handle<JobPromise<T>>::from_promise(*this));
        }

        void return_value(T value) noexcept {
            m_Value = value;
        }

        T Get() {
            return m_Value;
        }

        T m_Value{};
    };

    template <>
    struct JobPromise<void> : public JobPromiseBase<void> {
        JobPromise() noexcept
            : Axle::JobPromiseBase<void>(std::coroutine_handle<JobPromise<void>>::from_promise(*this)) {}

        JobCoroutine<void> get_return_object() {
            return JobCoroutine<void>(std::coroutine_handle<JobPromise<void>>::from_promise(*this));
        }

        void return_void() {}
    };

    struct JobCounterAwaiter {
        JobCounterAwaitable* m_Counter;

        JobCounterAwaiter(JobCounterAwaitable* counter)
            : m_Counter(counter) {}

        bool await_ready() const noexcept;

        template <typename TPromise>
        bool await_suspend(std::coroutine_handle<TPromise> handle) noexcept;

        void await_resume() const noexcept;
    };

    struct JobCounterAwaitable {
        std::atomic<u16> m_Count{0};
        // The job that is currently waiting
        std::atomic<Job*> m_Waiting{nullptr};

        void Increment() {
            m_Count.fetch_add(1, std::memory_order_release);
        }

        // Returns true if the decrement hits zero
        bool Decrement() {
            return m_Count.fetch_sub(1, std::memory_order_acq_rel) == 1;
        }

        bool IsDone() const {
            return m_Count.load(std::memory_order_acquire) == 1;
        }

        JobCounterAwaiter operator co_await() {
            return JobCounterAwaiter(this);
        }
    };

    bool JobCounterAwaiter::await_ready() const noexcept {
        // If already zero, don't suspend at all
        return m_Counter->IsDone();
    }

    template <typename TPromise>
    bool JobCounterAwaiter::await_suspend(std::coroutine_handle<TPromise> handle) noexcept {
        Job* job = &handle.promise();
        job->m_WaitingOn = m_Counter;

        // Store the job as the waiter
        Job* expected = nullptr;
        if (!m_Counter->m_Waiting.compare_exchange_strong(
                expected, job, std::memory_order_release, std::memory_order_acquire)) {
            // Something already waiting - shouldn't happen with single waiter model
            AX_PANIC("Counter already has a waiting job");
        }

        // Check for the race: counter may have hit zero between
        // await_ready returning false and us storing the waiter
        if (m_Counter->IsDone()) {
            m_Counter->m_Waiting.store(nullptr, std::memory_order_release);
            return false; // false = don't suspend, continue immediately
        }

        return true; // true = suspend
    }

    void JobCounterAwaiter::await_resume() const noexcept {}

    class JobSystem {
    public:
        JobSystem(const JobSystem& other) = delete;
        JobSystem& operator=(const JobSystem&) = delete;

        // Constructors do nothing because the initialization/destruction is manual with Init/ShutDown
        JobSystem() {}
        ~JobSystem() {}

        /**
         * Simple method for initialising the system. Shall not be called more than once.
         *
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         *
         * @param threadCount How many working threads you need. The render thread is excluded from this count. However,
         * the main one is included. So if you want 2 additional dedicated working threads you would pass a 3.
         *
         * Caution, this system depends on others, so they have to be initialized before this one.
         * */
        static void Init(ThreadAffinity threadCount) {
            if (s_JobSystem != nullptr) {
                AX_CORE_WARN("Init method of the JobSystem has been called a second time. IGNORING");
                return;
            }

            ThreadAffinity totalThreads = std::thread::hardware_concurrency();
            // FIX: This error message is temporal, in such cases simply prevent the initialization or something
            AX_ENSURE(totalThreads > 0, "There are not sufficient threads to initialize the job system");

            s_JobSystem = std::make_unique<JobSystem>();
            JobSystem& js = *s_JobSystem;

            js.m_NumThreads = std::min(totalThreads, threadCount);
            js.m_Running.store(true, std::memory_order_seq_cst);

            // Create local buffers
            js.m_JobLocalBuffers.reserve(js.m_NumThreads);
            for (ThreadAffinity i = 0; i < js.m_NumThreads; ++i) {
                js.m_JobLocalBuffers.emplace_back(std::make_unique<RingBuffer<Job*, BufferCapacity>>());
            }

            js.m_CVs.reserve(js.m_NumThreads);
            js.m_CVsMutex.reserve(js.m_NumThreads);
            for (ThreadAffinity i = 0; i < js.m_NumThreads; ++i) {
                js.m_CVsMutex.emplace_back(std::make_unique<std::mutex>());
                js.m_CVs.emplace_back(std::make_unique<std::condition_variable>());
            }

            // Spawn worker threads (skip 0, that's the main thread)
            for (ThreadAffinity i = 1; i < js.m_NumThreads; ++i) {
                js.m_Threads.emplace_back([i, &js]() {
                    js.SetupWorkerThread(i);
                    js.WorkerLoop();
                });
            }

            AX_INFO("Created {0} new worker threads (excluding the main one)", js.m_NumThreads - 1);

            // Main thread is also a worker
            js.SetupWorkerThread(0);

            AX_CORE_INFO("JobSystem initalized...");
        }

        /**
         * Same as init but for destroying and cleaning everything.
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         * */
        static void Shutdown() {
            if (s_JobSystem == nullptr) {
                AX_CORE_WARN("JobSystem Shutdown method was called more than once. IGNORING");
                return;
            }

            s_JobSystem->m_Running.store(false, std::memory_order_seq_cst);
            for (ThreadAffinity i = 0; i < s_JobSystem->m_NumThreads; ++i) {
                std::unique_lock lock(*(s_JobSystem->m_CVsMutex[i]));
                s_JobSystem->m_CVs[i]->notify_all();
            }

            // s_JobSystem->EmptyBuffer(); // drain main thread's buffer

            for (std::thread& thread : s_JobSystem->m_Threads) {
                if (thread.joinable())
                    thread.join();
            }

            s_JobSystem.reset();
            AX_CORE_INFO("JobSystem deleted...");
        }

        /**
         * Simply gets the singleton instance. Call after initialization.
         * */
        inline static JobSystem& GetInstance() noexcept {
            return *s_JobSystem;
        }

        // FIX: Optimize parameters to not make unnecessary copies
        void Schedule(JobFunction* job) {
            // The thread matters
            if (job->m_ThreadIndex != InvalidThreadIndex) {
                AX_ASSERT(job->m_ThreadIndex <= m_NumThreads, "Can't assign a job to a non existing thread");
                m_JobLocalBuffers[job->m_ThreadIndex]->Push(job);

                // Notify the needed thread
                std::scoped_lock lock(*m_CVsMutex[job->m_ThreadIndex]);
                m_CVs[job->m_ThreadIndex]->notify_all();
            }
            // Thread is irrelevant
            else {
                m_JobBuffers[static_cast<u8>(job->m_Priority)].enqueue(job);

                // Find idle thread via bitmask
                u64 idle = m_IdleThreads.load(std::memory_order_acquire);
                if (idle == 0)
                    return; // all busy, they'll pick it up when done

                int index = std::countr_zero(idle);
                {
                    std::scoped_lock lock(*m_CVsMutex[index]);
                    m_CVs[index]->notify_all();
                }
            }
        }

        void Schedule(Job* job) {
            // The thread matters
            if (job->m_ThreadIndex != InvalidThreadIndex) {
                AX_ASSERT(job->m_ThreadIndex <= m_NumThreads, "Can't assign a job to a non existing thread");
                m_JobLocalBuffers[job->m_ThreadIndex]->Push(job);

                // Notify the needed thread
                std::scoped_lock lock(*m_CVsMutex[job->m_ThreadIndex]);
                m_CVs[job->m_ThreadIndex]->notify_all();
            }
            // Thread is irrelevant
            else {
                m_JobBuffers[static_cast<u8>(job->m_Priority)].enqueue(job);

                // Find idle thread via bitmask
                u64 idle = m_IdleThreads.load(std::memory_order_acquire);
                if (idle == 0)
                    return; // all busy, they'll pick it up when done

                int index = std::countr_zero(idle);
                {
                    std::scoped_lock lock(*m_CVsMutex[index]);
                    m_CVs[index]->notify_all();
                }
            }
        }

        template <typename T>
        void Schedule(JobCoroutine<T>& job) {
            Schedule(job, InvalidThreadIndex, JobPriority::Medium);
        }
        template <typename T>
        void Schedule(JobCoroutine<T>& job, JobPriority priority) {
            Schedule(job, InvalidThreadIndex, priority);
        }
        template <typename T>
        void Schedule(JobCoroutine<T>& job, ThreadAffinity threadId) {
            Schedule(job, threadId, JobPriority::Medium);
        }
        template <typename T>
        void Schedule(JobCoroutine<T>& job, ThreadAffinity threadId, JobPriority priority) {
            job.m_Handle.promise().m_Priority = priority;
            job.m_Handle.promise().m_ThreadIndex = threadId;
            job.m_Scheduled = true;

            // The thread matters
            if (threadId != InvalidThreadIndex) {
                AX_ASSERT(threadId <= m_NumThreads, "Can't assign a job to a non existing thread");
                m_JobLocalBuffers[threadId]->Push(&job.m_Handle.promise());

                // Notify the needed tread
                std::scoped_lock lock(*m_CVsMutex[threadId]);
                m_CVs[threadId]->notify_all();
            } else {
                m_JobBuffers[static_cast<u8>(priority)].enqueue(&job.m_Handle.promise());

                // Find idle thread via bitmask
                u64 idle = m_IdleThreads.load(std::memory_order_acquire);
                if (idle == 0)
                    return; // all busy, they'll pick it up when done

                int index = std::countr_zero(idle);
                {
                    std::scoped_lock lock(*m_CVsMutex[index]);
                    m_CVs[index]->notify_all();
                }
            }

            // Release ownership over the handle so the frame doesn't get freed
            // job.Release();
        }

#ifdef AXLE_TESTING
        ThreadAffinity GetNumThreadsDEBUG() const {
            return m_NumThreads;
        }
        const std::vector<std::thread>& GetThreadsDEBUG() const {
            return m_Threads;
        }
        ThreadAffinity GetThreadIndexDEBUG() const {
            return m_Index;
        }
        u64 GetLocalBufferNumDEBUG() const {
            return m_JobLocalBuffers.size();
        }

#endif // AXLE_TESTING


    private:
        void SetupWorkerThread(ThreadAffinity threadId) {
            m_Index = threadId;
        }

        void WorkerLoop() {
            // We lock here because:
            // https://stackoverflow.com/questions/38147825/shared-atomic-variable-is-not-properly-published-if-it-is-not-modified-under-mut
            std::unique_lock lock(*m_CVsMutex[m_Index]);

            while (m_Running.load(std::memory_order_acquire)) {
                // Update bit mask
                m_IdleThreads.fetch_or(1ULL << m_Index, std::memory_order_release);
                m_CVs[m_Index]->wait(lock, [this] {
                    return !m_Running.load(std::memory_order_acquire) // Wake up to shutdown job system
                                                                      // Check local buffers
                           || !m_JobLocalBuffers[m_Index]->Empty() ||
                           // Check priority buffers
                           m_JobBuffers[2].size_approx() > 0 || m_JobBuffers[1].size_approx() > 0 ||
                           m_JobBuffers[0].size_approx() > 0;
                });
                // Update bit mask
                m_IdleThreads.fetch_and(~(1ULL << m_Index), std::memory_order_release);

                if (!m_Running.load(std::memory_order_acquire))
                    break;

                lock.unlock();
                // Run job
                RunPendingJob();
                lock.lock();
            }

            // Finish everything that remains so the thread can join
        }

        void RunPendingJob() {
            // First check local buffers
            Expected<Job*> jobLocalExp = m_JobLocalBuffers[m_Index]->Pop();
            if (jobLocalExp.IsValid()) {
                Job* job = jobLocalExp.Unwrap();

                if (job->m_IsFunction) {
                    static_cast<JobFunction*>(job)->m_Function();
                } else {
                    job->Resume(); // virtual call
                    // after resuming, check if this job completing unblocks someone
                    // this requires jobs to know which counter they decrement when done
                }

                return;
            }

            // Check global priority buffers in order of priority
            Job* job;
            for (int i = 2; i >= 0; --i) {
                // Found a coroutine job
                if (m_JobBuffers[i].try_dequeue(job)) {
                    if (job->m_IsFunction) {
                        static_cast<JobFunction*>(job)->m_Function();
                    } else {
                        job->Resume(); // virtual call
                        // after resuming, check if this job completing unblocks someone
                        // this requires jobs to know which counter they decrement when done
                    }
                    return;
                }
            }
        }

        void OnCounterZero(JobCounterAwaitable* counter) {
            Job* waiting = counter->m_Waiting.exchange(nullptr, std::memory_order_acq_rel);
            AX_ASSERT(waiting != nullptr, "The waiting job is null");
            if (waiting == nullptr)
                return;

            // Reset the counter for later use
            waiting->m_WaitingOn = nullptr;
            // re-queue it into the appropriate buffer
            Schedule(waiting);
        }

        inline static std::unique_ptr<JobSystem> s_JobSystem = nullptr;

        std::atomic_bool m_Running{false};
        ThreadAffinity m_NumThreads;

        std::array<moodycamel::ConcurrentQueue<Job*>, 3> m_JobBuffers;
        std::vector<JobBufferPtr<Job*>> m_JobLocalBuffers{};

        std::vector<std::thread> m_Threads;
        std::vector<std::unique_ptr<std::condition_variable>> m_CVs;
        std::vector<std::unique_ptr<std::mutex>> m_CVsMutex;

        inline static thread_local ThreadAffinity m_Index = InvalidThreadIndex;

        // 0 on an index: thread busy, 1: thread idle
        std::atomic<u64> m_IdleThreads{0};

        // Debug asserts
        static_assert(std::atomic<u64>::is_always_lock_free, "u64 is not always atomic");
        static_assert(std::atomic_bool::is_always_lock_free, "bool is not always atomic");
    };
} // namespace Axle
