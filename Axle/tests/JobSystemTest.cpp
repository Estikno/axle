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
    JobSystem::Init(3 // use 2 dedicated threads
    );
}

static void ShutdownJS() {
    JobSystem::Shutdown();
    // Log::ShutDown();
}

TEST_CASE("JobSystem - Created correctly") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    // Check dimensions
    REQUIRE(js.GetLocalBufferNumDEBUG() == 3);
    CHECK(JobSystem::GetInstance().GetThreadsDEBUG().size() == 2);
    CHECK(JobSystem::GetInstance().GetNumThreadsDEBUG() == 3);

    // If t_WorkerThread of the main thread is correct we assume everything else is also in order
    CHECK(js.GetThreadIndexDEBUG() == 0);

    ShutdownJS();
}

JobCoroutine<void> SimpleCoroutine(std::atomic<bool>* executed) {
    *executed = true;
    co_return;
}

JobCoroutine<int> SimpleCoroutineInt(std::atomic<bool>* executed) {
    executed->store(true);
    co_return 10;
}

JobCoroutine<int> SimpleCoroutineInt2() {
    co_return 10;
}

JobCoroutine<void> SimpleCoroutineVoid(std::atomic<bool>* excecuted, std::atomic<int>* val) {
    int a = co_await SimpleCoroutineInt2();
    val->store(a);
    excecuted->store(true);
    co_return;
}

JobCoroutine<void> SimpleCoroutineChangeThread(std::atomic<bool>* correct, ThreadAffinity thread) {
    co_await thread;
    correct->store(JobSystem::GetInstance().GetThreadIndexDEBUG() == thread);
    co_return;
}

// ─────────────────────────────────────────────
// Basic submission
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - fire and forget job executes") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<bool> executed{false};
    JobFunction* job = new JobFunction([&executed]() { executed = true; });

    CHECK_NOTHROW(js.Schedule(job));

    auto start = std::chrono::steady_clock::now();
    while (!executed) {
        std::this_thread::yield();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5)); // fail if takes too long
    }

    CHECK(executed);
    ShutdownJS();
}

TEST_CASE("JobSystem - fire and forget coroutine executes") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<bool> executed{false};
    JobCoroutine<void> job = SimpleCoroutine(&executed);

    CHECK_NOTHROW(js.Schedule(job));

    auto start = std::chrono::steady_clock::now();
    while (!executed) {
        std::this_thread::yield();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5)); // fail if takes too long
    }

    CHECK(executed);
    ShutdownJS();
}

TEST_CASE("JobSystem - coroutine that returns cannot be excecuted from a non-coroutine") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<bool> executed{false};
    JobCoroutine<int> job = SimpleCoroutineInt(&executed);

    CHECK_NOTHROW(js.Schedule(job));

    auto start = std::chrono::steady_clock::now();
    while (!executed) {
        std::this_thread::yield();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5)); // fail if takes too long
    }

    CHECK(executed);
    CHECK(!job.Get().IsValid());

    ShutdownJS();
}

TEST_CASE("JobSystem - coroutine that returns excecutes correctly") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<bool> executed{false};
    std::atomic<int> val{0};
    JobCoroutine<void> job = SimpleCoroutineVoid(&executed, &val);

    CHECK_NOTHROW(js.Schedule(job));

    auto start = std::chrono::steady_clock::now();
    while (!executed) {
        std::this_thread::yield();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5)); // fail if takes too long
    }

    CHECK(executed);
    CHECK_EQ(val.load(), 10);

    ShutdownJS();
}

TEST_CASE("JobSystem - coroutine that changes thread excecutes correctly") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<bool> correct{false};
    JobCoroutine<void> job = SimpleCoroutineChangeThread(&correct, 2);

    CHECK_NOTHROW(js.Schedule(job, 1));

    auto start = std::chrono::steady_clock::now();
    while (!correct) {
        std::this_thread::yield();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5)); // fail if takes too long
    }

    CHECK(correct);

    ShutdownJS();
}

// TEST_CASE("JobSystem - multiple fire and forget jobs all execute") {
//     InitJS();
//
//     constexpr int jobCount = 100;
//     std::atomic<int> counter{0};
//
//     for (int i = 0; i < jobCount; ++i) {
//         JobSystem::GetInstance().Submit([&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });
//     }
//
//     auto start = std::chrono::steady_clock::now();
//     while (counter < jobCount) {
//         bool ranJob = JobSystem::GetInstance().RunPendingJob();
//         if (!ranJob)
//             std::this_thread::yield();
//         auto elapsed = std::chrono::steady_clock::now() - start;
//         REQUIRE(elapsed < std::chrono::seconds(5));
//     }
//
//     CHECK(counter == jobCount);
//     ShutdownJS();
// }
//
// TEST_CASE("JobSystem - multiple fire and forget jobs all execute (more stressfull)") {
//     InitJS();
//
//     constexpr int jobCount = 10000;
//     std::atomic<int> counter{0};
//
//     for (int i = 0; i < jobCount; ++i) {
//         JobSystem::GetInstance().Submit([&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });
//     }
//
//     auto start = std::chrono::steady_clock::now();
//     while (counter < jobCount) {
//         bool ranJob = JobSystem::GetInstance().RunPendingJob();
//         if (!ranJob)
//             std::this_thread::yield();
//         auto elapsed = std::chrono::steady_clock::now() - start;
//         REQUIRE(elapsed < std::chrono::seconds(5));
//     }
//
//     CHECK(counter == jobCount);
//     ShutdownJS();
// }
//
// // ─────────────────────────────────────────────
// // JobFuture
// // ─────────────────────────────────────────────
//
// TEST_CASE("JobSystem - future returns correct value") {
//     InitJS();
//
//     auto future = JobSystem::GetInstance().Submit<int>([]() { return 42; });
//
//     int result = future.Wait();
//     CHECK(result == 42);
//
//     ShutdownJS();
// }
//
// TEST_CASE("JobSystem - future returns correct value from heavy computation") {
//     InitJS();
//
//     auto future = JobSystem::GetInstance().Submit<int>([]() {
//         int sum = 0;
//         for (int i = 0; i < 1000; ++i)
//             sum += i;
//         return sum;
//     });
//
//     CHECK(future.Wait() == 499500);
//     ShutdownJS();
// }
//
// TEST_CASE("JobSystem - multiple futures all resolve") {
//     InitJS();
//
//     constexpr int jobCount = 50;
//     std::vector<JobFuture<int>> futures;
//
//     for (int i = 0; i < jobCount; ++i) {
//         futures.push_back(JobSystem::GetInstance().Submit<int>([i]() { return i * 2; }));
//     }
//
//     for (int i = 0; i < jobCount; ++i)
//         CHECK(futures[i].Wait() == i * 2);
//
//     ShutdownJS();
// }
//
// TEST_CASE("JobSystem - multiple futures (a lot of them) all resolve") {
//     InitJS();
//
//     constexpr int jobCount = 200;
//     std::vector<JobFuture<int>> futures;
//
//     for (int i = 0; i < jobCount; ++i) {
//         futures.push_back(JobSystem::GetInstance().Submit<int>([i]() { return i * 2; }));
//     }
//
//     for (int i = 0; i < jobCount; ++i)
//         CHECK(futures[i].Wait() == i * 2);
//
//     ShutdownJS();
// }
//
// // ─────────────────────────────────────────────
// // Nested jobs
// // ─────────────────────────────────────────────
//
// TEST_CASE("JobSystem - job can submit child jobs") {
//     InitJS();
//
//     std::atomic<int> counter{0};
//
//     auto outer = JobSystem::GetInstance().Submit<void>([&counter]() {
//         auto& js = JobSystem::GetInstance();
//
//         auto a = js.Submit<void>([&counter]() { counter.fetch_add(1); });
//         auto b = js.Submit<void>([&counter]() { counter.fetch_add(1); });
//         auto c = js.Submit<void>([&counter]() { counter.fetch_add(1); });
//
//         a.Wait();
//         b.Wait();
//         c.Wait();
//     });
//
//     outer.Wait();
//     CHECK(counter == 3);
//     ShutdownJS();
// }
//
// TEST_CASE("JobSystem - nested futures resolve correctly") {
//     InitJS();
//
//     auto outer = JobSystem::GetInstance().Submit<int>([]() {
//         auto inner = JobSystem::GetInstance().Submit<int>([]() { return 10; });
//         return inner.Wait() * 2;
//     });
//
//     CHECK(outer.Wait() == 20);
//     ShutdownJS();
// }
//
// // ─────────────────────────────────────────────
// // Shutdown
// // ─────────────────────────────────────────────
//
// TEST_CASE("JobSystem - shutdown drains remaining jobs") {
//     InitJS();
//
//     std::atomic<int> counter{0};
//     constexpr int jobCount = 100;
//
//     for (int i = 0; i < jobCount; ++i) {
//         JobSystem::GetInstance().Submit([&counter]() { counter.fetch_add(1); });
//     }
//
//     JobSystem::GetInstance().Submit([&]() {
//         for (int i = 0; i < 500; i++) {
//             JobSystem::GetInstance().Submit([&counter]() { counter.fetch_add(1); });
//         }
//     });
//
//     // Shutdown should drain everything before returning
//     ShutdownJS();
//
//     CHECK(counter == 600);
// }
//
// TEST_CASE("JobSystem - double init is ignored") {
//     InitJS();
//     CHECK_NOTHROW(InitJS()); // should warn and return, not crash
//     ShutdownJS();
// }
//
// TEST_CASE("JobSystem - double shutdown is safe") {
//     InitJS();
//     ShutdownJS();
//     CHECK_NOTHROW(ShutdownJS());
// }
