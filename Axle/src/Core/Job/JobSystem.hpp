#pragma once

#include "Other/CustomTypes/Expected.hpp"
#include "axpch.hpp"
#include "JobBuffer.hpp"
#include "Core/Types.hpp"

namespace Axle {
    struct WorkerThread {
        std::shared_ptr<JobBuffer> m_LocalBuffer;
        std::vector<std::shared_ptr<JobBuffer>> m_StealBuffers;
        u32 m_Index;
    };

    // Forward declarations
    template <typename T>
    class SettableJobFuture;
    template <typename T>
    class JobFuture;

    /// Per thread variable that contains a pointer to the current WorkerThread struct of the thread
    inline thread_local WorkerThread* t_WorkerThread = nullptr;

    class JobSystem {
    public:
        /**
         * Simple method for initialising the system. Shall not be called more than once.
         *
         * Caution, this system depends on others, so they have to be initialized before this one.
         * */
        static void Init(u32 threadCount, u8 bufferCapacity, u32 renderBufferCapacity);

        /**
         * Same as init but for destroying and cleaning everything.
         * */
        static void Shutdown();

        /**
         * Simply gets the singleton instance. Call after initialization.
         * */
        inline static JobSystem& GetInstance() {
            return *s_JobSystem;
        }

        /**
         * Excecutes a pending job on the caller thread
         *
         * The job may be from the woker's own buffer or solen from others.
         * Render jobs are only to be taken by the render thread. As OpengGL is not Multi-Threaded.
         * */
        void RunPendingJob();

        /**
         * Submits a job to the buffer of the calling thread.
         * This job may be done by the same thread or others.
         *
         * Imporant: Do not submit render jobs via this method, use SubmitToRenderThread instead.
         * */
        void Submit(Job job);

        /**
         * Same as the void Submit method but here retrns a JobFuture, allowing you to wait until
         * the job is done.
         * */
        template <typename T>
        JobFuture<T> Submit(std::function<T()> job);

        /**
         * Special submit function for render jobs.
         *
         * This shall not be used for submitting non render jobs as render jobs can't be stolen by
         * other threads, so you might block the whole thread with non-render jobs. For this case use
         * either the void Submit or JobFuture Submit methods.
         * */
        void SubmitToRenderThread(Job job);

    private:
        // Singleton
        static std::unique_ptr<JobSystem> s_JobSystem;

        /**
         * Automate setting up worker threads.
         * */
        void SetupWorkerThread(u32 index, bool isRenderThread = false);

        /**
         * Defines the loop which every worker follows. That is, checking for available jobs
         * and doing them when possible.
         * */
        void WorkerLoop(u32 index);

        /**
         * The worker thread calling this method proceds to only doing jobs from their own buffer
         * until this one is empty.
         *
         * Usefull when shutting down the system and want to finish all remaining tasks.
         * */
        void EmptyBuffer();

        /**
         * Helper function that pops a job from the worker's thead own buffer.
         *
         * Also checks that the calling thread is in fact a working one and if that's not the case
         * returns immediately logging the error.
         *
         * @returns An Expected type with the job if it succeeded.
         * */
        Expected<Job> PopJob();

        /**
         * Helper funtion that steals a job from other worker threads buffer.
         *
         * Also checks that the calling thread is in fact a working one and if that's not the case
         * returns immediately logging the error.
         *
         * @returns An Expected type with the job if it succeeded.
         * */
        Expected<Job> StealJob();

        u32 m_NumThreads;
        u32 m_RenderThreadIndex;
        std::vector<std::thread> m_Threads;
        std::vector<std::shared_ptr<JobBuffer>> m_Buffers;
        std::atomic<bool> m_Running{false};
    };

    template <typename T>
    JobFuture<T> JobSystem::Submit(std::function<T()> job) {
        if (!t_WorkerThread) {
            AX_CORE_ERROR("Submit called from non-worker thread");
            // Still need to return something valid
            auto [settable, future] = SettableJobFuture<T>::Make();
            settable.Set(job()); // invoke immediately
            return future;
        }

        auto [settable, future] = SettableJobFuture<T>::Make();

        Job wrappedJob = [s = std::move(settable), j = std::move(job)]() mutable { s.Set(j()); };

        while (!t_WorkerThread->m_LocalBuffer->TryPush(wrappedJob))
            RunPendingJob();

        return future;
    }

    template <typename T>
    struct JobFutureInner {
        std::atomic<bool> isReady{false};
        std::optional<T> data;
        std::mutex mutex;
    };

    // Shared between the two halves
    template <typename T>
    using JobFutureInnerPtr = std::shared_ptr<JobFutureInner<T>>;

    template <typename T>
    class JobFuture {
    public:
        JobFuture(JobFuture&&) = default;
        JobFuture& operator=(JobFuture&&) = default;
        JobFuture(const JobFuture&) = delete;
        JobFuture& operator=(const JobFuture&) = delete;

        explicit JobFuture(JobFutureInnerPtr<T> inner)
            : m_Inner(std::move(inner)) {}

        T Wait() {
            while (true) {
                std::unique_lock<std::mutex> lock(m_Inner->mutex, std::try_to_lock);
                if (lock.owns_lock() && m_Inner->isReady) {
                    T value = std::move(*m_Inner->data);
                    m_Inner->data.reset();
                    return value;
                }
                // Keep the system productive while waiting
                JobSystem::GetInstance().RunPendingJob();
            }
        }

    private:
        JobFutureInnerPtr<T> m_Inner;
    };

    template <typename T>
    class SettableJobFuture {
    public:
        SettableJobFuture(SettableJobFuture&&) = default;
        SettableJobFuture& operator=(SettableJobFuture&&) = default;
        SettableJobFuture(const SettableJobFuture&) = delete;
        SettableJobFuture& operator=(const SettableJobFuture&) = delete;

        static std::pair<SettableJobFuture<T>, JobFuture<T>> Make() {
            auto inner = std::make_shared<JobFutureInner<T>>();
            return {SettableJobFuture<T>(inner), JobFuture<T>(inner)};
        }

        void Set(T result) {
            std::scoped_lock lock(m_Inner->mutex);
            m_Inner->data = std::move(result);
            m_Inner->isReady.store(true, std::memory_order_seq_cst);
        }

    private:
        explicit SettableJobFuture(JobFutureInnerPtr<T> inner)
            : m_Inner(std::move(inner)) {}
        JobFutureInnerPtr<T> m_Inner;
    };
} // namespace Axle
