#include "axpch.hpp"

#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "JobSystem.hpp"
#include "JobBuffer.hpp"

namespace Axle {
    std::unique_ptr<JobSystem> JobSystem::s_JobSystem;

    void JobSystem::Init(u32 threadCount, u8 bufferCapacity, u32 renderBufferCapacity) {
        if (s_JobSystem != nullptr) {
            AX_CORE_WARN("Init method of the JobSystem has been called a second time. IGNORING");
            return;
        }


        // -1 for the render thread (OpengGL is single threaded)
        u32 totalThreads = std::thread::hardware_concurrency() - 1;
        // FIX: This error message is temporal, in such cases simply prevent the initialization or something
        AX_ENSURE(totalThreads > 0, "There are not sufficient threads to initialize the job system");

        s_JobSystem = std::make_unique<JobSystem>();
        JobSystem& js = *s_JobSystem;

        js.m_NumThreads = std::min(totalThreads, threadCount);
        js.m_RenderThreadIndex = js.m_NumThreads;
        js.m_Running.store(true, std::memory_order_seq_cst);

        // Create one buffer per worker
        for (u32 i = 0; i < js.m_NumThreads; ++i)
            js.m_Buffers.push_back(std::make_shared<JobBuffer>(bufferCapacity));

        // Render thread buffer — larger since it can't fall back to immediate invocation
        js.m_Buffers.push_back(std::make_shared<JobBuffer>(renderBufferCapacity));

        // Spawn worker threads (skip 0, that's the main thread)
        for (u32 i = 1; i < js.m_NumThreads; ++i) {
            js.m_Threads.emplace_back([i, &js]() {
                js.SetupWorkerThread(i);
                js.WorkerLoop(i);
            });
        }

        // Spawn render thread
        js.m_Threads.emplace_back([&js]() {
            js.SetupWorkerThread(js.m_RenderThreadIndex, true);
            js.WorkerLoop(js.m_RenderThreadIndex);
        });

        AX_INFO("Created {0} new worker threads (excluding the main one)", js.m_NumThreads);

        // Main thread is also a worker
        js.SetupWorkerThread(0);
    }

    void JobSystem::Shutdown() {
        if (s_JobSystem == nullptr) {
            AX_CORE_WARN("JobSystem Shutdown method was called more than once. IGNORING");
            return;
        }

        s_JobSystem->m_Running.store(false, std::memory_order_seq_cst);
        s_JobSystem->EmptyBuffer(); // drain main thread's buffer

        for (auto& thread : s_JobSystem->m_Threads)
            if (thread.joinable())
                thread.join();

        delete t_WorkerThread; // clean up main thread's WorkerThread
        t_WorkerThread = nullptr;
        s_JobSystem.reset();
    }

    void JobSystem::SetupWorkerThread(u32 index, bool isRenderThread) {
        // Store basic info
        t_WorkerThread = new WorkerThread();
        t_WorkerThread->m_Index = index;
        t_WorkerThread->m_LocalBuffer = m_Buffers[index];

        AX_CORE_INFO("Hello from thread {0}", index);

        // Computes stealing buffers
        for (u32 i = 1; i <= m_NumThreads; ++i) {
            u32 targetIndex = (index + i) % (m_NumThreads + 1);

            if (!isRenderThread && targetIndex == m_RenderThreadIndex)
                continue;

            t_WorkerThread->m_StealBuffers.push_back(m_Buffers[targetIndex]);
        }
    }

    void JobSystem::WorkerLoop(u32 index) {
        while (m_Running.load(std::memory_order_seq_cst))
            RunPendingJob();

        EmptyBuffer();
        delete t_WorkerThread;
        t_WorkerThread = nullptr;
    }

    void JobSystem::EmptyBuffer() {
        while (true) {
            auto result = PopJob();
            if (!result.IsValid())
                break;
            result.Unwrap()();
        }
    }

    Expected<Job> JobSystem::PopJob() {
        if (!t_WorkerThread) {
            AX_CORE_ERROR("PopJob called from non-worker thread");
            return Expected<Job>::FromException(std::runtime_error("Not a worker thread"));
        }

        // Render thread steals (blocking just like Pop) to preserve submission order
        if (t_WorkerThread->m_Index == m_RenderThreadIndex) {
            return t_WorkerThread->m_LocalBuffer->Steal();
        }

        return t_WorkerThread->m_LocalBuffer->Pop();
    }

    Expected<Job> JobSystem::StealJob() {
        if (!t_WorkerThread) {
            AX_CORE_ERROR("StealJob called from non-worker thread");
            return Expected<Job>::FromException(std::runtime_error("Not a worker thread"));
        }

        for (auto& buffer : t_WorkerThread->m_StealBuffers) {
            auto result = buffer->TrySteal();
            if (result.IsValid())
                return result;
        }
        return Expected<Job>::FromException(std::runtime_error("Nothing to steal"));
    }

    void JobSystem::RunPendingJob() {
        auto popped = PopJob();
        if (popped.IsValid()) {
            popped.Unwrap()();
            return;
        }

        auto stolen = StealJob();
        if (stolen.IsValid()) {
            stolen.Unwrap()();
            return;
        }

        std::this_thread::yield();
    }

    void JobSystem::SubmitToRenderThread(Job job) {
        if (!t_WorkerThread) {
            AX_CORE_ERROR(
                "SubmitToRenderThread called from a non-worker thread. Cannot excecute the job because OpengGL works exclusively on its render thread.");
            return;
        }

        // Block until there is space — render jobs cannot be invoked on the wrong thread
        while (!m_Buffers[m_RenderThreadIndex]->TryPush(job))
            RunPendingJob(); // stay productive while waiting for space
    }

    void JobSystem::Submit(Job job) {
        if (!t_WorkerThread) {
            AX_CORE_ERROR("Submit called from non-worker thread");
            job();
            return;
        }

        // FIX: If the render thread submits a non render related job it will still go to it's render buffer

        while (!t_WorkerThread->m_LocalBuffer->TryPush(job))
            RunPendingJob();
    }

    // void specialization
    // ------------------
    template <>
    JobFuture<void> JobSystem::Submit(std::function<void()> job) {
        if (!t_WorkerThread) {
            AX_CORE_ERROR("Submit called from non-worker thread");
            auto [settable, future] = SettableJobFuture<void>::Make();
            job();
            settable.Set();
            return std::move(future);
        }

        auto [settable, future] = SettableJobFuture<void>::Make();

        auto sharedState = std::make_shared<std::pair<SettableJobFuture<void>, std::function<void()>>>(
            std::move(settable), std::move(job));

        Job wrappedJob = [sharedState]() mutable {
            sharedState->second();
            sharedState->first.Set();
        };

        while (!t_WorkerThread->m_LocalBuffer->TryPush(wrappedJob))
            RunPendingJob();

        return std::move(future);
    }
    // ------------------
} // namespace Axle
