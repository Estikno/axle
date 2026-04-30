#include <doctest.h>

#include "Core/Job/JobSystem.hpp"
#include "Core/Logger/Log.hpp"
#include <atomic>
#include <chrono>
#include <thread>

using namespace Axle;

// ─────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────

static void InitJS() {
    Log::Init();
    JobSystem::Init(3,  // use 2 dedicated threads
                    64, // buffer capacity
                    128 // render buffer capacity
    );
}

static void ShutdownJS() {
    JobSystem::Shutdown();
    // Log::ShutDown();
}

TEST_CASE("JobSystem - Created correctly") {
    InitJS();

    std::vector<std::shared_ptr<JobBuffer>>& buffers = JobSystem::GetInstance().GetBuffers();

    // Check dimensions
    REQUIRE(buffers.size() == 4);
    CHECK(buffers[0]->GetSize() == 64);
    CHECK(buffers[1]->GetSize() == 64);
    CHECK(buffers[2]->GetSize() == 64);
    CHECK(buffers[3]->GetSize() == 128);

    CHECK(JobSystem::GetInstance().GetThreads().size() == 3);

    CHECK(JobSystem::GetInstance().GetNumThreads() == 3);
    CHECK(JobSystem::GetInstance().GetNumThreads() == JobSystem::GetInstance().GetRenderIndex());

    // If t_WorkerThread of the main thread is correct we assume everything else is also in order
    CHECK(t_WorkerThread->m_Index == 0);
    CHECK(t_WorkerThread->m_LocalBuffer.get() == buffers[0].get());
    REQUIRE(t_WorkerThread->m_StealBuffers.size() == 2);
    CHECK(t_WorkerThread->m_StealBuffers[0].get() == buffers[1].get());
    CHECK(t_WorkerThread->m_StealBuffers[1].get() == buffers[2].get());

    ShutdownJS();
}

// ─────────────────────────────────────────────
// Basic submission
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - fire and forget job executes") {
    InitJS();

    std::atomic<bool> executed{false};
    CHECK_NOTHROW(JobSystem::GetInstance().Submit([&executed]() { executed = true; }));

    // Help process while waiting
    auto start = std::chrono::steady_clock::now();
    while (!executed) {
        JobSystem::GetInstance().RunPendingJob();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5)); // fail if takes too long
    }

    CHECK(executed);
    ShutdownJS();
}

TEST_CASE("JobSystem - multiple fire and forget jobs all execute") {
    InitJS();

    constexpr int jobCount = 100;
    std::atomic<int> counter{0};

    for (int i = 0; i < jobCount; ++i) {
        JobSystem::GetInstance().Submit([&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });
    }

    auto start = std::chrono::steady_clock::now();
    while (counter < jobCount) {
        JobSystem::GetInstance().RunPendingJob();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5));
    }

    CHECK(counter == jobCount);
    ShutdownJS();
}

TEST_CASE("JobSystem - multiple fire and forget jobs all execute (more stressfull)") {
    InitJS();

    constexpr int jobCount = 10000;
    std::atomic<int> counter{0};

    for (int i = 0; i < jobCount; ++i) {
        JobSystem::GetInstance().Submit([&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });
    }

    auto start = std::chrono::steady_clock::now();
    while (counter < jobCount) {
        JobSystem::GetInstance().RunPendingJob();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5));
    }

    CHECK(counter == jobCount);
    ShutdownJS();
}

// ─────────────────────────────────────────────
// JobFuture
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - future returns correct value") {
    InitJS();

    auto future = JobSystem::GetInstance().Submit<int>([]() { return 42; });

    int result = future.Wait();
    CHECK(result == 42);

    ShutdownJS();
}

TEST_CASE("JobSystem - future returns correct value from heavy computation") {
    InitJS();

    auto future = JobSystem::GetInstance().Submit<int>([]() {
        int sum = 0;
        for (int i = 0; i < 1000; ++i)
            sum += i;
        return sum;
    });

    CHECK(future.Wait() == 499500);
    ShutdownJS();
}

TEST_CASE("JobSystem - multiple futures all resolve") {
    InitJS();

    constexpr int jobCount = 50;
    std::vector<JobFuture<int>> futures;

    for (int i = 0; i < jobCount; ++i) {
        futures.push_back(JobSystem::GetInstance().Submit<int>([i]() { return i * 2; }));
    }

    for (int i = 0; i < jobCount; ++i)
        CHECK(futures[i].Wait() == i * 2);

    ShutdownJS();
}

TEST_CASE("JobSystem - multiple futures (a lot of them) all resolve") {
    InitJS();

    constexpr int jobCount = 200;
    std::vector<JobFuture<int>> futures;

    for (int i = 0; i < jobCount; ++i) {
        futures.push_back(JobSystem::GetInstance().Submit<int>([i]() { return i * 2; }));
    }

    for (int i = 0; i < jobCount; ++i)
        CHECK(futures[i].Wait() == i * 2);

    ShutdownJS();
}

// ─────────────────────────────────────────────
// Nested jobs
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - job can submit child jobs") {
    InitJS();

    std::atomic<int> counter{0};

    auto outer = JobSystem::GetInstance().Submit<void>([&counter]() {
        auto& js = JobSystem::GetInstance();

        auto a = js.Submit<void>([&counter]() { counter.fetch_add(1); });
        auto b = js.Submit<void>([&counter]() { counter.fetch_add(1); });
        auto c = js.Submit<void>([&counter]() { counter.fetch_add(1); });

        a.Wait();
        b.Wait();
        c.Wait();
    });

    outer.Wait();
    CHECK(counter == 3);
    ShutdownJS();
}

TEST_CASE("JobSystem - nested futures resolve correctly") {
    InitJS();

    auto outer = JobSystem::GetInstance().Submit<int>([]() {
        auto inner = JobSystem::GetInstance().Submit<int>([]() { return 10; });
        return inner.Wait() * 2;
    });

    CHECK(outer.Wait() == 20);
    ShutdownJS();
}

// ─────────────────────────────────────────────
// Render thread
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - render thread executes submitted jobs") {
    InitJS();

    std::atomic<bool> executed{false};
    JobSystem::GetInstance().SubmitToRenderThread([&]() {
        executed = true;
        // Verify it's the actual render thread
        REQUIRE(t_WorkerThread->m_Index == 3);
    });

    auto start = std::chrono::steady_clock::now();
    while (!executed) {
        JobSystem::GetInstance().RunPendingJob();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5));
    }

    CHECK(executed);
    ShutdownJS();
}

TEST_CASE("JobSystem - render thread executes all submitted jobs") {
    InitJS();

    constexpr int jobCount = 50;
    std::atomic<int> done{0};

    for (int i = 0; i < jobCount; ++i) {
        JobSystem::GetInstance().SubmitToRenderThread([&done]() { done.fetch_add(1); });
    }

    auto start = std::chrono::steady_clock::now();
    while (done < jobCount) {
        JobSystem::GetInstance().RunPendingJob();
        REQUIRE(std::chrono::steady_clock::now() - start < std::chrono::seconds(5));
    }

    CHECK(done == jobCount);
    ShutdownJS();
}

TEST_CASE("JobSystem - render thread jobs execute in order") {
    InitJS();

    constexpr int jobCount = 20;
    std::vector<int> order;
    std::mutex orderMutex;
    std::atomic<int> done{0};

    for (int i = 0; i < jobCount; ++i) {
        JobSystem::GetInstance().SubmitToRenderThread([i, &order, &orderMutex, &done]() {
            {
                std::scoped_lock lock(orderMutex);
                order.push_back(i);
                // simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            done.fetch_add(1);
        });
    }

    auto start = std::chrono::steady_clock::now();
    while (done < jobCount) {
        JobSystem::GetInstance().RunPendingJob();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(10));
    }

    // Render jobs should execute in submission order since it's a single thread
    for (int i = 0; i < jobCount; ++i)
        CHECK(order[i] == i);

    ShutdownJS();
}

// ─────────────────────────────────────────────
// Shutdown
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - shutdown drains remaining jobs") {
    InitJS();

    std::atomic<int> counter{0};
    constexpr int jobCount = 100;

    for (int i = 0; i < jobCount; ++i) {
        JobSystem::GetInstance().Submit([&counter]() { counter.fetch_add(1); });
    }

    JobSystem::GetInstance().SubmitToRenderThread([&]() {
        for (int i = 0; i < 500; i++) {
            JobSystem::GetInstance().Submit([&counter]() { counter.fetch_add(1); });
        }
    });

    // Shutdown should drain everything before returning
    ShutdownJS();

    CHECK(counter == 600);
}

TEST_CASE("JobSystem - double init is ignored") {
    InitJS();
    CHECK_NOTHROW(InitJS()); // should warn and return, not crash
    ShutdownJS();
}

TEST_CASE("JobSystem - double shutdown is safe") {
    InitJS();
    ShutdownJS();
    CHECK_NOTHROW(ShutdownJS());
}
