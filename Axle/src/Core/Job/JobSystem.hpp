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

    inline thread_local WorkerThread* t_WorkerThread = nullptr;

    class JobSystem {
    public:
        static void Init(u32 threadCount, u8 bufferCapacity, u32 renderBufferCapacity);
        static void Shutdown();
        inline static JobSystem& GetInstance() {
            return *s_JobSystem;
        }

        void RunPendingJob();
        void Submit(Job job);
        template <typename T>
        JobFuture<T> Submit(std::function<T()> job);
        void SubmitToRenderThread(Job job);

    private:
        static std::unique_ptr<JobSystem> s_JobSystem;

        void SetupWorkerThread(u32 index, bool isRenderThread = false);
        void WorkerLoop(u32 index);
        void EmptyBuffer();

        Expected<Job> PopJob();
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
