#pragma once

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
    template <typename T>
    struct FinalAwaiter;
    template <typename T, typename U>
    struct JobAwaiter;
    template <typename T, typename... Us>
    struct JobAwaiterMultiple;
    template <typename T>
    struct ThreadAwaiter;

    struct Job {
        JobPriority m_Priority{JobPriority::Medium};
        ThreadAffinity m_ThreadIndex{InvalidThreadIndex};
        bool m_IsFunction{true};
        Job* m_Parent{nullptr};         // Who is waiting for me
        std::atomic<u32> m_Children{0}; // How many children I'm waiting for

        Job() = default;
        Job(JobPriority priority, ThreadAffinity threadIndex, bool isFunction)
            : m_Priority(priority),
              m_ThreadIndex(threadIndex),
              m_IsFunction(isFunction) {}
        virtual ~Job() = default;

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

        std::coroutine_handle<JobPromise<T>> m_Handle{};
        bool m_Scheduled{false};

        JobCoroutine() = default;
        explicit JobCoroutine(std::coroutine_handle<JobPromise<T>> h)
            : m_Handle(h) {}

        JobCoroutine(JobCoroutine&& other) noexcept
            : m_Handle(std::exchange(other.m_Handle, {})),
              m_Scheduled(std::exchange(other.m_Scheduled, false)) {}
        JobCoroutine& operator=(JobCoroutine&& other) noexcept {
            if (this != &other) {
                // Destroy our current handle if we own one
                if (m_Handle)
                    m_Handle.destroy();
                m_Handle = std::exchange(other.m_Handle, {});
                m_Scheduled = std::exchange(other.m_Scheduled, false);
            }
            return *this;
        }
        JobCoroutine(const JobCoroutine&) = delete;
        ~JobCoroutine() {
            if (!m_Handle)
                return;

            if (!m_Scheduled) {
                m_Handle.destroy();
                return;
            }

            // If scheduled as a child (has parent) and is done,
            // we are responsible for cleanup since FinalAwaiter returned true
            if (m_Handle.done() && m_Handle.promise().m_Parent != nullptr) {
                m_Handle.destroy();
                return;
            }

            // If scheduled without parent, FinalAwaiter self-destructed, don't touch it
        }

        Expected<T> Get() noexcept
            requires(!std::is_void_v<T>)
        {
            if (m_Handle && m_Handle.done())
                return m_Handle.promise().Get();
            return Expected<T>::FromException(
                "Trying to get a value out of a non-existing coroutine or a non-finished one");
        }
    };

    template <typename... Us>
    struct WhenAllTag {
        std::tuple<JobCoroutine<Us>...> coros;
    };

    template <typename... Us>
    WhenAllTag<Us...> WhenAll(JobCoroutine<Us>&&... coros) {
        return {std::tuple<JobCoroutine<Us>...>(std::move(coros)...)};
    }

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
        FinalAwaiter<T> final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {
            AX_PANIC("Coroutines in the JobSystem panicked");
            // std::terminate();
        }

        template <typename U>
        JobAwaiter<T, U> await_transform(JobCoroutine<U>& cor) {
            return JobAwaiter<T, U>(cor);
        }

        template <typename U>
        JobAwaiter<T, U> await_transform(JobCoroutine<U>&& cor) {
            return JobAwaiter<T, U>(std::move(cor));
        }

        template <typename... Us>
        JobAwaiterMultiple<T, Us...> await_transform(WhenAllTag<Us...>&& tag) {
            return std::apply([](auto&&... coros) { return JobAwaiterMultiple<T, Us...>(std::move(coros)...); },
                              std::move(tag.coros));
        }

        ThreadAwaiter<T> await_transform(ThreadAffinity thread) {
            return ThreadAwaiter<T>(thread);
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
                // If no parent will be set (top-level schedule), null the handle
                // so the destructor can't touch the freed frame after FinalAwaiter runs
                job.m_Handle = {};

                // Notify the needed tread
                std::scoped_lock lock(*m_CVsMutex[threadId]);
                m_CVs[threadId]->notify_all();
            } else {
                m_JobBuffers[static_cast<u8>(priority)].enqueue(&job.m_Handle.promise());
                // If no parent will be set (top-level schedule), null the handle
                // so the destructor can't touch the freed frame after FinalAwaiter runs
                job.m_Handle = {};

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
        template <typename T>
        friend struct FinalAwaiter;

        template <typename T, typename U>
        friend struct JobAwaiter;

        template <typename T, typename... Us>
        friend struct JobAwaiterMultiple;

        template <typename T>
        friend struct ThreadAwaiter;

        void Schedule(Job* job, Job* parent) {
            job->m_Parent = parent;

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
                RunJob(job);
                return;
            }

            // Check global priority buffers in order of priority
            Job* job;
            for (int i = 2; i >= 0; --i) {
                // Found a coroutine job
                if (m_JobBuffers[i].try_dequeue(job)) {
                    RunJob(job);
                    return;
                }
            }
        }

        void RunJob(Job* job) {
            if (job->m_IsFunction) {
                static_cast<JobFunction*>(job)->m_Function();
                // Delete the function job since it can't possibly be rescheduled
                delete job;
            } else {
                job->Resume(); // virtual call
            }
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

    // The awaiter tests whether there is a parent (i.e. a coro) - if yes then the parent should destroy this coro.
    // The parent might need the return value, which is stored in the promise of this coro.
    //
    // If there is no parent, because this coro was scheduled at the start from the main function,
    // then the coro destroys itself.
    template <typename T>
    struct FinalAwaiter : public std::suspend_always {
        bool await_suspend(std::coroutine_handle<JobPromise<T>> handle) noexcept {
            Job* parent = handle.promise().m_Parent;

            if (parent != nullptr) {
                u32 remaining = parent->m_Children.fetch_sub(1, std::memory_order_acq_rel);
                if (remaining == 1) {
                    // We were the last child, wake parent up
                    JobSystem::GetInstance().Schedule(parent, parent->m_Parent);
                }
                return true; // stay alive, parent will destroy us after reading value
            }
            return false; // no parent, self-destruct
        }
    };

    template <typename T>
    struct ThreadAwaiter {
        ThreadAffinity m_Thread;

        ThreadAwaiter<T>(ThreadAffinity thread)
            : m_Thread(thread) {}

        bool await_ready() noexcept {
            // Don't suspend if we are already on the wanted thread
            return JobSystem::GetInstance().m_Index == m_Thread;
        }

        void await_suspend(std::coroutine_handle<JobPromise<T>> h) noexcept {
            Job* job = &h.promise();
            job->m_ThreadIndex = m_Thread;

            JobSystem::GetInstance().Schedule(job, job->m_Parent);
        }

        void await_resume() noexcept {}
    };

    template <typename T, typename U>
    struct JobAwaiter {
        JobCoroutine<U> m_Coro;     // owned copy (rvalue case)
        JobPromise<U>* m_JobToWait; // non-owning (lvalue case)

        // Lvalue version - cor is kept alive by the caller
        JobAwaiter<T, U>(JobCoroutine<U>& cor)
            : m_Coro{},
              m_JobToWait(&cor.m_Handle.promise()) {}

        // Rvalue version - takes ownership so the temporary stays alive
        JobAwaiter<T, U>(JobCoroutine<U>&& cor)
            : m_Coro(std::move(cor)),
              m_JobToWait(nullptr) // will use m_Coro directly
        {}

        // Delete move/copy to prevent pointer invalidation
        JobAwaiter(const JobAwaiter&) = delete;
        JobAwaiter& operator=(const JobAwaiter&) = delete;
        JobAwaiter(JobAwaiter&&) = delete;
        JobAwaiter& operator=(JobAwaiter&&) = delete;

        JobPromise<U>* GetPromise() {
            // If we own the coro, reach through it; otherwise use the raw pointer
            if (m_Coro.m_Handle)
                return &m_Coro.m_Handle.promise();
            return m_JobToWait;
        }

        bool await_ready() noexcept {
            return false;
        }

        bool await_suspend(std::coroutine_handle<JobPromise<T>> h) noexcept {
            Job* parent = &h.promise();
            // Schedule the job
            parent->m_Children.fetch_add(1, std::memory_order_release);
            JobSystem::GetInstance().Schedule(GetPromise(), parent);
            return true;
        }

        U await_resume() noexcept {
            if constexpr (!std::is_void_v<U>) {
                return GetPromise()->Get();
                // cor's destructor will handle Destroy()
            }
        }
    };

    template <typename T, typename... Us>
    struct JobAwaiterMultiple {
        std::tuple<JobCoroutine<Us>...> m_Coros;

        JobAwaiterMultiple(JobCoroutine<Us>&&... coros)
            : m_Coros(std::move(coros)...) {}

        // Delete move/copy for same reason as JobAwaiter
        JobAwaiterMultiple(const JobAwaiterMultiple&) = delete;
        JobAwaiterMultiple& operator=(const JobAwaiterMultiple&) = delete;
        JobAwaiterMultiple(JobAwaiterMultiple&&) = delete;
        JobAwaiterMultiple& operator=(JobAwaiterMultiple&&) = delete;

        bool await_ready() noexcept {
            return false;
        }

        bool await_suspend(std::coroutine_handle<JobPromise<T>> h) noexcept {
            Job* parent = &h.promise();

            // Increment children
            parent->m_Children.fetch_add(sizeof...(Us), std::memory_order_release);

            // Schedule all children
            std::apply(
                [&](auto&... coros) { (JobSystem::GetInstance().Schedule(&coros.m_Handle.promise(), parent), ...); },
                m_Coros);

            return true;
        }

        auto await_resume() noexcept {
            return std::apply([&](auto&... coros) { return std::make_tuple(GetValue(coros)...); }, m_Coros);
        }

        // Helper to get value or void_t placeholder
        template <typename U>
        auto GetValue(JobCoroutine<U>& coro) {
            if constexpr (std::is_void_v<U>)
                return std::monostate{};
            else
                return coro.m_Handle.promise().Get();
        }
    };
} // namespace Axle
