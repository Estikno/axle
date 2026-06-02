#pragma once

#include "Other/CustomTypes/Expected.hpp"
#include <coroutine>

#include "axpch.hpp"
#include "RingBuffer.hpp"

namespace Axle {
    inline static constexpr u32 BufferCapacity = 64;
    using ThreadAffinity = u8;
    enum class JobPriority { Low = 0, Medium, High };
    template <typename T>
    using JobBufferPtr = std::unique_ptr<JobBuffer<T, BufferCapacity>>;

    // Forward declarations
    struct Job;
    struct JobCoroutine;
    struct JobPromise;
    struct JobFunction;

    struct Job {
        JobPriority m_Priority{JobPriority::Medium};
        ThreadAffinity m_ThreadIndex{0};
        bool m_IsFunction{true};

        Job() = default;
        Job(JobPriority priority, u32 threadIndex)
            : m_Priority(priority),
              m_ThreadIndex(threadIndex) {}

        virtual void Resume() = 0;
        virtual void Destroy() = 0;
    };

    struct JobFunction : public Job {
        std::function<void()> m_Function;

        void Resume() override {
            m_Function();
        }

        void Destroy() override {}
    };

    struct JobCoroutine {
        std::coroutine_handle<JobPromise> m_Handle;

        explicit JobCoroutine(std::coroutine_handle<JobPromise> h)
            : m_Handle(h) {}
        JobCoroutine(JobCoroutine&& other) noexcept
            : m_Handle(other.m_Handle) {
            other.m_Handle = {};
        }
        JobCoroutine(const JobCoroutine&) = delete;
        ~JobCoroutine() {
            if (m_Handle)
                m_Handle.destroy();
        }

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
            js.m_JobCorLocalBuffers.resize(js.m_NumThreads);
            js.m_JobFunLocalBuffers.resize(js.m_NumThreads);

            js.m_CVs.resize(js.m_NumThreads);
            js.m_CVsMutex.resize(js.m_NumThreads);

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
            for (auto& cv : s_JobSystem->m_CVs) {
                cv->notify_all();
            }

            // s_JobSystem->EmptyBuffer(); // drain main thread's buffer

            for (std::thread& thread : s_JobSystem->m_Threads) {
                if (thread.joinable())
                    thread.join();
            }

            s_JobSystem.reset();
            AX_CORE_INFO("JobSystem deleted...");
        }

    private:
        void SetupWorkerThread(ThreadAffinity threadId) {
            m_Index = threadId;
        }

        void WorkerLoop() {
            while (m_Running.load(std::memory_order_acquire)) {
                // Run jobs

                std::unique_lock lock(*m_CVsMutex[m_Index]);
                m_CVs[m_Index]->wait(lock);
            }

            // Finish everything that remains
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

            // Check global priority buffers
        }


        inline static std::unique_ptr<JobSystem> s_JobSystem;

        std::atomic_bool m_Running{false};
        ThreadAffinity m_NumThreads;

        std::array<JobBuffer<JobCoroutine, BufferCapacity>, 3> m_JobCorBuffers{};
        std::vector<JobBufferPtr<JobCoroutine>> m_JobCorLocalBuffers{};

        std::array<JobBuffer<JobFunction, BufferCapacity>, 3> m_JobFunBuffers{};
        std::vector<JobBufferPtr<JobFunction>> m_JobFunLocalBuffers{};

        std::vector<std::thread> m_Threads;
        std::vector<std::unique_ptr<std::condition_variable>> m_CVs;
        std::vector<std::unique_ptr<std::mutex>> m_CVsMutex;

        inline static thread_local ThreadAffinity m_Index;
    };
} // namespace Axle
