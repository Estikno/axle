#pragma once

#include "axpch.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "JobBuffer.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Types.hpp"

// JobSystem based on Rismoch blog: https://www.rismosch.com/article?id=building-a-job-system

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
         * @param threadCount How many working threads you need. The render thread is excluded from this count. However,
         * the main one is included. So if you want 2 additional dedicated working threads you would pass a 3.
         * @param bufferCapacity How many jobs are each working thread going to be able to store.
         * @param renderBufferCapacity Same as bufferCapacity but for the render thread. It's recommended to make the
         * render buffer larger than the normal ones.
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
         * @param Job The job to be done
         *
         * Imporant: Do not submit render jobs via this method, use SubmitToRenderThread instead.
         * */
        void Submit(Job job);

        /**
         * Same as the void Submit method but here retrns a JobFuture, allowing you to wait until
         * the job is done.
         *
         * @param Job The job to be done
         * */
        template <typename T>
        JobFuture<T> Submit(std::function<T()> job);

        /**
         * Special submit function for render jobs.
         *
         * This shall not be used for submitting non render jobs as render jobs can't be stolen by
         * other threads, so you might block the whole thread with non-render jobs. For this case use
         * either the void Submit or JobFuture Submit methods.
         *
         * @param Job The job to be done
         * */
        void SubmitToRenderThread(Job job);

#ifdef AXLE_TESTING
        std::vector<std::shared_ptr<JobBuffer>>& GetBuffers() {
            return m_Buffers;
        }

        std::vector<std::thread>& GetThreads() {
            return m_Threads;
        }

        u32 GetNumThreads() {
            return m_NumThreads;
        }

        u32 GetRenderIndex() {
            return m_RenderThreadIndex;
        }
#endif // AXLE_TESTING

    private:
        // Singleton
        static std::unique_ptr<JobSystem> s_JobSystem;

        /**
         * Automate setting up worker threads.
         *
         * @param index Which worker index to setup
         * @param isRenderThread Indicates if the thread to be set up is the working thread or not
         * */
        void SetupWorkerThread(u32 index, bool isRenderThread = false);

        /**
         * Defines the loop which every worker follows. That is, checking for available jobs
         * and doing them when possible.
         *
         * @param index Which worker index to setup
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

        /// Stores the number of working threads (exlcuding the Render one)
        u32 m_NumThreads;
        /// Index of the render thread in the m_Buffers vector
        u32 m_RenderThreadIndex;
        /// All additional threads spwaned
        std::vector<std::thread> m_Threads;
        /// All job buffers
        std::vector<std::shared_ptr<JobBuffer>> m_Buffers;
        /// Handy variable to help shutting down the system
        std::atomic<bool> m_Running{false};
    };

    template <typename T>
    JobFuture<T> JobSystem::Submit(std::function<T()> job) {
        if (!t_WorkerThread) {
            AX_CORE_ERROR("Submit called from non-worker thread");

            // Still need to return something valid
            auto [settable, future] = SettableJobFuture<T>::Make();
            settable.Set(job()); // invoke immediately
            return std::move(future);
        }

        auto [settable, future] = SettableJobFuture<T>::Make();

        // Wrap in shared_ptr so the lambda is copyable
        auto sharedState =
            std::make_shared<std::pair<SettableJobFuture<T>, std::function<T()>>>(std::move(settable), std::move(job));

        Job wrappedJob = [sharedState]() mutable { sharedState->first.Set(sharedState->second()); };

        while (!t_WorkerThread->m_LocalBuffer->TryPush(wrappedJob))
            RunPendingJob();

        return std::move(future);
    }

    /**
     * A simple wrapper around some data used in the job future logic
     * */
    template <typename T>
    struct JobFutureInner {
        bool isReady{false};
        std::optional<T> data;
        std::mutex mutex;
    };

    // Shared between the two halves
    template <typename T>
    using JobFutureInnerPtr = std::shared_ptr<JobFutureInner<T>>;

    /**
     * This is what the user waiting for the job to be done has.
     * It's a simple class that allows you to wait until the job is done.
     * */
    template <typename T>
    class JobFuture {
    public:
        JobFuture(JobFuture&&) = default;
        JobFuture& operator=(JobFuture&&) = default;
        JobFuture(const JobFuture&) = delete;
        JobFuture& operator=(const JobFuture&) = delete;

        explicit JobFuture(JobFutureInnerPtr<T> inner)
            : m_Inner(std::move(inner)) {}

        /**
         * Simply whaits for the job to be done and returns the data once the job has finished.
         *
         * If the calling thread is a worker one it will continue working in the meantime, so no busy waiting.
         * However, it will basically busy wait if the calling thread is not a working one.
         *
         * @returns The value that the job returns
         * */
        T Wait() {
            while (true) {
                {
                    std::scoped_lock lock(m_Inner->mutex);
                    if (m_Inner->isReady) {
                        T value = std::move(*m_Inner->data);
                        m_Inner->data.reset();
                        return value;
                    }
                } // Release the lock

                // Keep the system productive while waiting
                JobSystem::GetInstance().RunPendingJob();
            }
        }

    private:
        // Shared state
        JobFutureInnerPtr<T> m_Inner;
    };

    /**
     * This is what the thread doing the job gets. It allows notifying the user that the job has been done and set the
     * returning data.
     * */
    template <typename T>
    class SettableJobFuture {
    public:
        SettableJobFuture(SettableJobFuture&&) = default;
        SettableJobFuture& operator=(SettableJobFuture&&) = default;
        SettableJobFuture(const SettableJobFuture&) = delete;
        SettableJobFuture& operator=(const SettableJobFuture&) = delete;

        /**
         * Simple function that facilitates creating the two needee classes.
         *
         * @returns A pair conatining in the first position the SettableJobFuture (thread side) and in the second
         * position the JobFuture (user side)
         * */
        static std::pair<SettableJobFuture<T>, JobFuture<T>> Make() {
            auto inner = std::make_shared<JobFutureInner<T>>();
            return {SettableJobFuture<T>(inner), JobFuture<T>(inner)};
        }

        /**
         * Marks the job as completed and moves the result into the data slot.
         *
         * @param result The data to be returned
         * */
        void Set(T result) {
            std::scoped_lock lock(m_Inner->mutex);
            m_Inner->data = std::move(result);
            m_Inner->isReady = true;
        }

    private:
        explicit SettableJobFuture(JobFutureInnerPtr<T> inner)
            : m_Inner(std::move(inner)) {}
        JobFutureInnerPtr<T> m_Inner;
    };

    // void specialization
    // ------------------
    template <>
    struct JobFutureInner<void> {
        bool isReady{false};
        std::mutex mutex;
        // no data field needed
    };

    template <>
    class JobFuture<void> {
    public:
        JobFuture(JobFuture&&) = default;
        JobFuture& operator=(JobFuture&&) = default;
        JobFuture(const JobFuture&) = delete;
        JobFuture& operator=(const JobFuture&) = delete;

        explicit JobFuture(JobFutureInnerPtr<void> inner)
            : m_Inner(std::move(inner)) {}

        void Wait() {
            while (true) {
                {
                    std::scoped_lock lock(m_Inner->mutex);
                    if (m_Inner->isReady)
                        return;
                }
                JobSystem::GetInstance().RunPendingJob();
            }
        }

    private:
        JobFutureInnerPtr<void> m_Inner;
    };

    template <>
    class SettableJobFuture<void> {
    public:
        SettableJobFuture(SettableJobFuture&&) = default;
        SettableJobFuture& operator=(SettableJobFuture&&) = default;
        SettableJobFuture(const SettableJobFuture&) = delete;
        SettableJobFuture& operator=(const SettableJobFuture&) = delete;

        static std::pair<SettableJobFuture<void>, JobFuture<void>> Make() {
            auto inner = std::make_shared<JobFutureInner<void>>();
            return {SettableJobFuture<void>(inner), JobFuture<void>(inner)};
        }

        void Set() {
            std::scoped_lock lock(m_Inner->mutex);
            m_Inner->isReady = true;
        }

    private:
        explicit SettableJobFuture(JobFutureInnerPtr<void> inner)
            : m_Inner(std::move(inner)) {}
        JobFutureInnerPtr<void> m_Inner;
    };

    template <>
    JobFuture<void> JobSystem::Submit(std::function<void()> job);
    // ------------------
} // namespace Axle
