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
    CHECK(JobSystem::GetInstance().GetNumThreads() == 3);

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

JobCoroutine<int> SimpleCoroutineWaitForMany2(std::atomic<int>* n) {
    n->fetch_add(1);
    co_return 12;
}

JobCoroutine<void> SimpleCoroutineWaitForMany1(std::atomic<int>* n) {
    n->fetch_add(1);
    co_return;
}

JobCoroutine<void> SimpleCoroutineWaitForMany(std::atomic<int>* n, std::atomic<int>* ret1) {
    auto [a, b] = co_await WhenAll(SimpleCoroutineWaitForMany1(n), SimpleCoroutineWaitForMany2(n));
    ret1->store(b);
    co_return;
}

// ─────────────────────────────────────────────
// Basic submission
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - fire and forget job executes") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<bool> executed{false};

    CHECK_NOTHROW(js.Schedule([&executed]() { executed = true; }));

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

TEST_CASE("JobSystem - coroutine that waits multiple jobs simulatenously excecutes correctly") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<int> n{0};
    std::atomic<int> ret1{0};
    JobCoroutine<void> job = SimpleCoroutineWaitForMany(&n, &ret1);

    CHECK_NOTHROW(js.Schedule(job));

    auto start = std::chrono::steady_clock::now();
    while (ret1.load() == 0) {
        std::this_thread::yield();
        auto elapsed = std::chrono::steady_clock::now() - start;
        REQUIRE(elapsed < std::chrono::seconds(5)); // fail if takes too long
    }

    CHECK_EQ(n.load(), 2);
    CHECK_EQ(ret1.load(), 12);

    ShutdownJS();
}

// ─────────────────────────────────────────────
// Priority and ordering
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - high priority job executes before low priority") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    // Fill threads with blocking jobs so we can queue up prioritized work
    std::atomic<bool> gate{false};
    std::atomic<int> executionOrder{0};
    std::atomic<int> lowOrder{-1};
    std::atomic<int> highOrder{-1};

    // Block all threads first
    for (int i = 0; i < 2; ++i) {
        js.Schedule([&gate]() {
            while (!gate.load(std::memory_order_acquire))
                std::this_thread::yield();
        });
    }

    // Small sleep to let blocking jobs occupy threads
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Queue low then high
    js.Schedule([&]() { lowOrder.store(executionOrder.fetch_add(1)); }, JobPriority::Low);
    js.Schedule([&]() { highOrder.store(executionOrder.fetch_add(1)); }, JobPriority::High);

    // Release the gate
    gate.store(true, std::memory_order_release);

    auto start = std::chrono::steady_clock::now();
    while (lowOrder.load() == -1 || highOrder.load() == -1) {
        std::this_thread::yield();
        REQUIRE(std::chrono::steady_clock::now() - start < std::chrono::seconds(5));
    }

    CHECK(highOrder.load() < lowOrder.load());

    ShutdownJS();
}

// ─────────────────────────────────────────────
// Thread affinity
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - function job runs on correct thread") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<bool> done{false};
    std::atomic<ThreadAffinity> ranOnThread{InvalidThreadIndex};

    js.Schedule(
        [&]() {
            ranOnThread.store(js.GetThreadIndexDEBUG());
            done.store(true);
        },
        JobPriority::Medium,
        2);

    auto start = std::chrono::steady_clock::now();
    while (!done.load()) {
        std::this_thread::yield();
        REQUIRE(std::chrono::steady_clock::now() - start < std::chrono::seconds(5));
    }

    CHECK_EQ(ranOnThread.load(), 2);

    ShutdownJS();
}

// ─────────────────────────────────────────────
// Stress tests
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - many function jobs all execute") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    constexpr int JobCount = 1000;
    std::atomic<int> counter{0};

    for (int i = 0; i < JobCount; ++i)
        js.Schedule([&counter]() { counter.fetch_add(1); });

    auto start = std::chrono::steady_clock::now();
    while (counter.load() < JobCount) {
        std::this_thread::yield();
        REQUIRE(std::chrono::steady_clock::now() - start < std::chrono::seconds(10));
    }

    CHECK_EQ(counter.load(), JobCount);

    ShutdownJS();
}

TEST_CASE("JobSystem - many coroutine jobs all execute") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    constexpr int JobCount = 100;
    std::atomic<int> counter{0};

    // Need to keep coroutines alive until scheduled
    std::vector<JobCoroutine<void>> jobs;
    jobs.reserve(JobCount);

    for (int i = 0; i < JobCount; ++i) {
        jobs.push_back([](std::atomic<int>* c) -> JobCoroutine<void> {
            c->fetch_add(1);
            co_return;
        }(&counter));
    }

    for (auto& job : jobs)
        js.Schedule(job);

    auto start = std::chrono::steady_clock::now();
    while (counter.load() < JobCount) {
        std::this_thread::yield();
        REQUIRE(std::chrono::steady_clock::now() - start < std::chrono::seconds(10));
    }

    CHECK_EQ(counter.load(), JobCount);

    ShutdownJS();
}

// ─────────────────────────────────────────────
// Chaining and nesting
// ─────────────────────────────────────────────

JobCoroutine<int> ChainedLevel2() {
    co_return 42;
}

JobCoroutine<int> ChainedLevel1() {
    int val = co_await ChainedLevel2();
    co_return val + 1;
}

JobCoroutine<void> ChainedLevel0(std::atomic<int>* result, std::atomic<bool>* done) {
    int val = co_await ChainedLevel1();
    result->store(val);
    done->store(true);
    co_return;
}

TEST_CASE("JobSystem - deeply chained coroutines resolve correctly") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<int> result{0};
    std::atomic<bool> done{false};
    JobCoroutine<void> job = ChainedLevel0(&result, &done);

    js.Schedule(job);

    auto start = std::chrono::steady_clock::now();
    while (!done.load()) {
        std::this_thread::yield();
        REQUIRE(std::chrono::steady_clock::now() - start < std::chrono::seconds(5));
    }

    CHECK_EQ(result.load(), 43);

    ShutdownJS();
}

JobCoroutine<void> WhenAllNested(std::atomic<int>* sum, std::atomic<bool>* done) {
    auto [a, b, c] = co_await WhenAll(ChainedLevel2(), ChainedLevel1(), ChainedLevel2());
    sum->store(a + b + c);
    done->store(true);
    co_return;
}

TEST_CASE("JobSystem - WhenAll with three jobs resolves correctly") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    std::atomic<int> sum{0};
    std::atomic<bool> done{false};
    JobCoroutine<void> job = WhenAllNested(&sum, &done);

    js.Schedule(job);

    auto start = std::chrono::steady_clock::now();
    while (!done.load()) {
        std::this_thread::yield();
        REQUIRE(std::chrono::steady_clock::now() - start < std::chrono::seconds(5));
    }

    // ChainedLevel2 = 42, ChainedLevel1 = 43, ChainedLevel2 = 42 → 127
    CHECK_EQ(sum.load(), 127);

    ShutdownJS();
}

// ─────────────────────────────────────────────
// Edge cases
// ─────────────────────────────────────────────

TEST_CASE("JobSystem - coroutine scheduled but never awaited doesn't leak") {
    // This test just checks it doesn't crash or leak - ASAN will catch leaks
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    {
        // Created but never scheduled - destructor should clean up
        JobCoroutine<int> unscheduled = ChainedLevel2();
        (void) unscheduled;
    }

    ShutdownJS();
}

TEST_CASE("JobSystem - multiple independent coroutines run concurrently") {
    InitJS();
    JobSystem& js = JobSystem::GetInstance();

    constexpr int N = 4;
    std::atomic<int> counter{0};
    std::atomic<bool> done[N];
    for (auto& d : done)
        d.store(false);

    auto makeJob = [](std::atomic<int>* c, std::atomic<bool>* d) -> JobCoroutine<void> {
        c->fetch_add(1);
        d->store(true);
        co_return;
    };

    std::vector<JobCoroutine<void>> jobs;
    for (int i = 0; i < N; ++i)
        jobs.push_back(makeJob(&counter, &done[i]));

    for (auto& job : jobs)
        js.Schedule(job);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
        while (!done[i].load()) {
            std::this_thread::yield();
            REQUIRE(std::chrono::steady_clock::now() - start < std::chrono::seconds(5));
        }
    }

    CHECK_EQ(counter.load(), N);

    ShutdownJS();
}
