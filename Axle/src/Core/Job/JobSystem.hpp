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
    struct JobCoroutine;
    struct JobPromise;
    struct JobFunction;

    struct Job {
        JobPriority m_Priority{JobPriority::Medium};
        ThreadAffinity m_ThreadIndex{InvalidThreadIndex};
        // bool m_IsFunction{true};

        Job() = default;
        Job(JobPriority priority, u32 threadIndex)
            : m_Priority(priority),
              m_ThreadIndex(threadIndex) {}

        virtual void Resume() = 0;
        virtual void Destroy() = 0;
    };

    struct JobFunction : public Job {
        std::function<void()> m_Function;

        JobFunction() = default;
        JobFunction(std::function<void()> func)
            : m_Function(func) {}

        void Resume() override {
            m_Function();
        }

        void Destroy() override {}
    };

    struct JobCoroutine {
        using promise_type = JobPromise;

        std::coroutine_handle<JobPromise> m_Handle;

        JobCoroutine() = default;
        explicit JobCoroutine(std::coroutine_handle<JobPromise> h)
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
        JobCoroutine(const JobCoroutine&) = default;
        // JobCoroutine(const JobCoroutine&) = delete;
        // ~JobCoroutine() {
        //     if (m_Handle)
        //         m_Handle.destroy();
        // }

        void resume() {
            if (m_Handle && !m_Handle.done())
                m_Handle.resume();
        }
    };

    struct JobPromise : public Job {
        JobCoroutine get_return_object() {
            return JobCoroutine(std::coroutine_handle<JobPromise>::from_promise(*this));
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void return_void() {}
        void unhandled_exception() {
            std::terminate();
        }

        virtual void Resume() override {}
        virtual void Destroy() override {}
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
            js.m_JobCorLocalBuffers.reserve(js.m_NumThreads);
            js.m_JobFunLocalBuffers.reserve(js.m_NumThreads);
            for (ThreadAffinity i = 0; i < js.m_NumThreads; ++i) {
                js.m_JobCorLocalBuffers.emplace_back(std::make_unique<RingBuffer<JobCoroutine, BufferCapacity>>());
                js.m_JobFunLocalBuffers.emplace_back(std::make_unique<RingBuffer<JobFunction, BufferCapacity>>());
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
        void Schedule(JobFunction job) {
            // The thread matters
            if (job.m_ThreadIndex != InvalidThreadIndex) {
                AX_ASSERT(job.m_ThreadIndex <= m_NumThreads, "Can't assign a job to a non existing thread");
                m_JobFunLocalBuffers[job.m_ThreadIndex]->Push(job);

                // Notify the needed thread
                std::scoped_lock lock(*m_CVsMutex[job.m_ThreadIndex]);
                m_CVs[job.m_ThreadIndex]->notify_all();
            }
            // Thread is irrelevant
            else {
                m_JobFunBuffers[static_cast<u8>(job.m_Priority)].enqueue(job);

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

        void Schedule(JobCoroutine job) {
            Schedule(job, InvalidThreadIndex, JobPriority::Medium);
        }
        void Schedule(JobCoroutine job, JobPriority priority) {
            Schedule(job, InvalidThreadIndex, priority);
        }
        void Schedule(JobCoroutine job, ThreadAffinity threadId) {
            Schedule(job, threadId, JobPriority::Medium);
        }
        void Schedule(JobCoroutine job, ThreadAffinity threadId, JobPriority priority) {
            job.m_Handle.promise().m_Priority = priority;
            job.m_Handle.promise().m_ThreadIndex = threadId;

            // The thread matters
            if (threadId != InvalidThreadIndex) {
                AX_ASSERT(threadId <= m_NumThreads, "Can't assign a job to a non existing thread");
                m_JobCorLocalBuffers[threadId]->Push(job);

                // Notify the needed tread
                std::scoped_lock lock(*m_CVsMutex[threadId]);
                m_CVs[threadId]->notify_all();
            } else {
                m_JobCorBuffers[static_cast<u8>(priority)].enqueue(job);

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
            return m_JobCorLocalBuffers.size();
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
                           || !m_JobCorLocalBuffers[m_Index]->Empty() || !m_JobFunLocalBuffers[m_Index]->Empty() ||
                           // Check priority buffers
                           m_JobCorBuffers[2].size_approx() > 0 || m_JobCorBuffers[1].size_approx() > 0 ||
                           m_JobCorBuffers[0].size_approx() > 0 || m_JobFunBuffers[2].size_approx() > 0 ||
                           m_JobFunBuffers[1].size_approx() > 0 || m_JobFunBuffers[0].size_approx() > 0;
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
            Expected<JobFunction> jobFun = m_JobFunLocalBuffers[m_Index]->Pop();
            if (jobFun.IsValid()) {
                jobFun.Unwrap().m_Function();
                return;
            }

            Expected<JobCoroutine> jobCor = m_JobCorLocalBuffers[m_Index]->Pop();
            if (jobCor.IsValid()) {
                jobCor.Unwrap().resume();
                return;
            }

            // Check global priority buffers in order of priority
            JobCoroutine cor{};
            JobFunction fun{};
            for (int i = 2; i >= 0; --i) {
                // Found a coroutine job
                if (m_JobCorBuffers[i].try_dequeue(cor)) {
                    cor.resume();
                    return;
                }

                // Found simple function job
                if (m_JobFunBuffers[i].try_dequeue(fun)) {
                    fun.m_Function();
                    return;
                }
            }
        }

        inline static std::unique_ptr<JobSystem> s_JobSystem = nullptr;

        std::atomic_bool m_Running{false};
        ThreadAffinity m_NumThreads;

        std::array<moodycamel::ConcurrentQueue<JobCoroutine>, 3> m_JobCorBuffers;
        std::vector<JobBufferPtr<JobCoroutine>> m_JobCorLocalBuffers{};

        std::array<moodycamel::ConcurrentQueue<JobFunction>, 3> m_JobFunBuffers;
        std::vector<JobBufferPtr<JobFunction>> m_JobFunLocalBuffers{};

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
