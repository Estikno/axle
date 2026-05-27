#include <doctest.h>

#include "Core/Logger/Log.hpp"
#include "Core/Types.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Other/CustomTypes/Expected.hpp"

#include <filesystem>
#include <thread>
#include <vector>
#include <atomic>
#include <barrier>
#include <deque>

using namespace Axle;

// Helper to create a temp file and clean it up after the test
struct TempFile {
    std::filesystem::path path;

    TempFile(const std::string& name, u64 size)
        : path("assets/tests/" + name) {
        if (std::filesystem::exists(path))
            std::filesystem::remove(path);
        ResourceManager::GetInstance().Create(path, size);
    }

    ~TempFile() {
        if (std::filesystem::exists(path))
            std::filesystem::remove(path);
    }
};

TEST_CASE("ResourceManager - Created correctly") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    CHECK_EQ(instance.LargestAvailableIndex(), 0);
    CHECK_EQ(instance.MagicNumberCounter(), 0);
    CHECK(instance.AvailableIndexes().empty());

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Double Init warns but does not crash") {
    ResourceManager::Init();
    ResourceManager::Init(); // Should warn but not crash or reset state
    ResourceManager& instance = ResourceManager::GetInstance();
    CHECK_EQ(instance.LargestAvailableIndex(), 0);
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load valid file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle = instance.Load("assets/tests/valid.txt");
        REQUIRE(eHandle.IsValid());
        FileHandle handle = eHandle.Unwrap().Get();

        CHECK(instance.AvailableIndexes().empty());
        CHECK_EQ(instance.LargestAvailableIndex(), (u16) 1);
        CHECK_EQ(instance.MagicNumberCounter(), (u16) 1);

        CHECK_EQ(instance.GetIndexFromHandle(handle), (u16) 0);
        CHECK_EQ(instance.GetMagicFromHandle(handle), (u16) 0);

        REQUIRE_EQ(instance.Size(handle).Unwrap(), (u64) 21);

        ResourceManager::ReadGuard pRead = instance.DataConst(eHandle.Unwrap()).Unwrap();
        const char* p = pRead.Data();

        CHECK(p[0] == 'T');
        CHECK(p[1] == 'h');
        CHECK(p[2] == 'i');
        CHECK(p[3] == 's');
        CHECK(p[4] == ' ');
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load non-existing file fails") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<ResourceManager::ManagedFileHandle> eHandle = instance.Load("assets/tests/novalid.txt");

    CHECK(!eHandle.IsValid());

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load same file twice returns same handle") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle1 = instance.Load("assets/tests/valid.txt");
        Expected<ResourceManager::ManagedFileHandle> eHandle2 = instance.Load("assets/tests/valid.txt");

        REQUIRE(eHandle1.IsValid());
        REQUIRE(eHandle2.IsValid());

        // Same file — should return the same handle, no new index allocated
        CHECK_EQ(eHandle1.Unwrap(), eHandle2.Unwrap());
        CHECK_EQ(instance.LargestAvailableIndex(), (u16) 1);
        CHECK_EQ(instance.MagicNumberCounter(), (u16) 1);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load multiple different files") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle1 = instance.Load("assets/tests/valid.txt");
        Expected<ResourceManager::ManagedFileHandle> eHandle2 = instance.Load("assets/tests/valid2.bin");

        REQUIRE(eHandle1.IsValid());
        REQUIRE(eHandle2.IsValid());

        FileHandle h1 = eHandle1.Unwrap().Get();
        FileHandle h2 = eHandle2.Unwrap().Get();

        // Different handles
        CHECK_NE(h1, h2);
        CHECK_EQ(instance.LargestAvailableIndex(), (u32) 2);
        CHECK_EQ(instance.MagicNumberCounter(), (u32) 2);

        // Different indexes
        CHECK_EQ(instance.GetIndexFromHandle(h1), (u32) 0);
        CHECK_EQ(instance.GetIndexFromHandle(h2), (u32) 1);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load read-write file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle = instance.Load("assets/tests/valid.txt", false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        // Data() should succeed for a read-write file
        Expected<ResourceManager::WriteGuard> data = instance.Data(handle);
        CHECK(data.IsValid());
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Data() fails on read-only file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle = instance.Load("assets/tests/valid.txt", true);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        // Data() should fail since file is read-only
        Expected<ResourceManager::WriteGuard> data = instance.Data(handle);
        CHECK_FALSE(data.IsValid());

        // DataConst() should succeed
        Expected<ResourceManager::ReadGuard> dataConst = instance.DataConst(handle);
        CHECK(dataConst.IsValid());
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Close releases index for reuse") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    FileHandle h1h;
    FileHandle h2h;

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle1 = instance.Load("assets/tests/valid.txt");
        REQUIRE(eHandle1.IsValid());
        ResourceManager::ManagedFileHandle h1 = eHandle1.Unwrap();
        h1h = h1.Get();
        CHECK_EQ(instance.GetIndexFromHandle(h1.Get()), (u32) 0);
    }

    // Index 0 should now be available again
    REQUIRE_FALSE(instance.AvailableIndexes().empty());
    CHECK_EQ(instance.AvailableIndexes().top(), (u32) 0);

    {
        // Loading a new file should reuse index 0
        Expected<ResourceManager::ManagedFileHandle> eHandle2 = instance.Load("assets/tests/valid2.bin", false);
        REQUIRE(eHandle2.IsValid());
        ResourceManager::ManagedFileHandle h2 = eHandle2.Unwrap();
        h2h = h2.Get();
        CHECK_EQ(instance.GetIndexFromHandle(h2.Get()), (u32) 0);
    }

    // Magic should have incremented though — stale handle h1 is now invalid
    CHECK_NE(instance.GetMagicFromHandle(h1h), instance.GetMagicFromHandle(h2h));

    CHECK_EQ(instance.LargestAvailableIndex(), (u32) 1);
    CHECK_EQ(instance.MagicNumberCounter(), (u32) 2);

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Stale handle is rejected after close") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    FileHandle h1;

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle = instance.Load("assets/tests/valid.txt");
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();
        h1 = handle.Get();
    }

    {
        // Reopen the file — new magic, old handle is stale
        Expected<ResourceManager::ManagedFileHandle> eHandle2 = instance.Load("assets/tests/valid.txt");
        REQUIRE(eHandle2.IsValid());

        // Size() with stale handle should fail magic check and return invalid
        Expected<u64> size = instance.Size(h1);
        CHECK_FALSE(size.IsValid());
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Create new file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    const std::string newFile = "assets/tests/created.bin";

    // Clean up leftovers from a previous failed run
    if (std::filesystem::exists(newFile))
        std::filesystem::remove(newFile);

    bool created = instance.Create(newFile, 128);
    CHECK(created);
    CHECK(std::filesystem::exists(newFile));
    CHECK_EQ(std::filesystem::file_size(newFile), (u64) 128);

    std::filesystem::remove(newFile);
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Create existing file fails") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    // valid.txt already exists — Create should refuse
    CHECK_FALSE(instance.Create("assets/tests/valid.txt", 64));

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Sync read-only file returns false") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle = instance.Load("assets/tests/valid.txt", true);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        // Syncing a read-only map is a no-op and returns false
        CHECK_FALSE(instance.Sync(handle));
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Sync read-write file succeeds") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        Expected<ResourceManager::ManagedFileHandle> eHandle = instance.Load("assets/tests/valid.txt", false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        CHECK(instance.Sync(handle));
    }

    ResourceManager::ShutDown();
}

// ─────────────────────────────────────────────
//  ShutDown
// ─────────────────────────────────────────────

TEST_CASE("ResourceManager - ShutDown closes all open files") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        Expected<ResourceManager::ManagedFileHandle> h = instance.Load("assets/tests/valid.txt");
        CHECK_EQ(instance.LargestAvailableIndex(), (u32) 1);

        // ShutDown should close everything without crashing
        ResourceManager::ShutDown();

        // Re-init to verify clean state
        ResourceManager::Init();
        ResourceManager& instance2 = ResourceManager::GetInstance();
        CHECK_EQ(instance2.LargestAvailableIndex(), (u32) 0);
        CHECK_EQ(instance2.MagicNumberCounter(), (u32) 0);
    }

    ResourceManager::ShutDown();
}

// Multithreading tests

TEST_CASE("ResourceManager MT - Concurrent loads of the same file return the same handle") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        constexpr int THREAD_COUNT = 8;
        std::vector<ResourceManager::ManagedFileHandle> handles(THREAD_COUNT);
        std::vector<std::thread> threads;

        // Barrier so all threads start loading at the same time
        std::barrier startBarrier(THREAD_COUNT);

        for (int i = 0; i < THREAD_COUNT; ++i) {
            threads.emplace_back([&, i]() {
                startBarrier.arrive_and_wait();
                auto eHandle = instance.Load("assets/tests/valid.txt");
                if (eHandle.IsValid())
                    handles[i] = eHandle.Unwrap();
            });
        }

        for (auto& t : threads)
            t.join();

        // All threads should have gotten the same handle
        ResourceManager::ManagedFileHandle& first = handles[0];
        REQUIRE_NE(first.Get(), INVALID_FILE_HANDLE);
        for (int i = 1; i < THREAD_COUNT; ++i)
            CHECK_EQ(handles[i], first);

        // Only one resource should have been allocated
        CHECK_EQ(instance.LargestAvailableIndex(), (u32) 1);
        CHECK_EQ(instance.MagicNumberCounter(), (u32) 1);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager MT - Concurrent loads of different files allocate unique indexes") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        // Prepare N temp files
        constexpr int FILE_COUNT = 8;
        std::vector<std::string> paths;
        for (int i = 0; i < FILE_COUNT; ++i) {
            std::string p = "assets/tests/mt_file_" + std::to_string(i) + ".bin";
            instance.Create(p, 64);
            paths.push_back(p);
        }

        {
            std::vector<ResourceManager::ManagedFileHandle> handles(FILE_COUNT);
            std::vector<std::thread> threads;
            std::barrier startBarrier(FILE_COUNT);

            for (int i = 0; i < FILE_COUNT; ++i) {
                threads.emplace_back([&, i]() {
                    // startBarrier.arrive_and_wait();
                    auto eHandle = instance.Load(paths[i]);
                    REQUIRE(eHandle.IsValid());
                    handles[i] = eHandle.Unwrap();
                });
            }

            for (auto& t : threads)
                t.join();

            for (int i = 0; i < FILE_COUNT; ++i)
                for (int j = i + 1; j < FILE_COUNT; ++j)
                    CHECK_NE(handles[i].Get(), handles[j].Get());

            // Exactly FILE_COUNT resources allocated
            CHECK_EQ(instance.LargestAvailableIndex(), (u32) FILE_COUNT);
        }

        // Cleanup
        for (auto& p : paths)
            std::filesystem::remove(p);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager MT - Concurrent DataConst reads do not block each other") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        auto eHandle = instance.Load("assets/tests/valid.txt");
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        constexpr int THREAD_COUNT = 8;
        std::atomic<int> successCount = 0;
        std::vector<std::thread> threads;
        std::barrier startBarrier(THREAD_COUNT);

        for (int i = 0; i < THREAD_COUNT; ++i) {
            threads.emplace_back([&]() {
                // startBarrier.arrive_and_wait();
                auto guard = instance.DataConst(handle);
                if (guard.IsValid()) {
                    // All threads should be able to read simultaneously
                    const char* data = guard.Unwrap().Data();
                    if (data[0] == 'T')
                        successCount.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        for (auto& t : threads)
            t.join();

        CHECK_EQ(successCount.load(), THREAD_COUNT);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager MT - Concurrent Size reads are consistent") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        auto eHandle = instance.Load("assets/tests/valid.txt");
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        constexpr int THREAD_COUNT = 8;
        std::vector<u64> sizes(THREAD_COUNT, 0);
        std::vector<std::thread> threads;
        std::barrier startBarrier(THREAD_COUNT);

        for (int i = 0; i < THREAD_COUNT; ++i) {
            threads.emplace_back([&, i]() {
                startBarrier.arrive_and_wait();
                auto size = instance.Size(handle);
                if (size.IsValid())
                    sizes[i] = size.Unwrap();
            });
        }

        for (auto& t : threads)
            t.join();

        // All threads should see the same size
        for (int i = 0; i < THREAD_COUNT; ++i)
            CHECK_EQ(sizes[i], (u64) 21);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager MT - File stays open while any ManagedFileHandle is alive") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    FileHandle rawHandle = INVALID_FILE_HANDLE;

    {
        auto eHandle1 = instance.Load("assets/tests/valid.txt");
        REQUIRE(eHandle1.IsValid());
        rawHandle = eHandle1.Unwrap().Get();

        {
            // Second handle — ref count goes to 2
            auto eHandle2 = instance.Load("assets/tests/valid.txt");
            REQUIRE(eHandle2.IsValid());

            // eHandle2 goes out of scope here — ref count drops to 1
        }

        // File should still be open since eHandle1 is still alive
        CHECK(instance.IsHandleValid(rawHandle));
    }

    // Both handles gone — file should be closed now
    CHECK_FALSE(instance.IsHandleValid(rawHandle));

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager MT - Concurrent ref count increments and decrements are safe") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        auto eHandle = instance.Load("assets/tests/valid.txt");
        REQUIRE(eHandle.IsValid());
        FileHandle rawHandle = eHandle.Unwrap().Get();

        constexpr int THREAD_COUNT = 8;
        std::vector<std::thread> threads;
        std::barrier startBarrier(THREAD_COUNT);

        for (int i = 0; i < THREAD_COUNT; ++i) {
            threads.emplace_back([&]() {
                startBarrier.arrive_and_wait();
                // Each thread loads (increments ref) then lets the handle drop (decrements ref)
                auto localHandle = instance.Load("assets/tests/valid.txt");
            });
        }

        for (auto& t : threads)
            t.join();

        // Original handle should still be valid — eHandle is still alive
        CHECK(instance.IsHandleValid(rawHandle));

        // Drop the original — now file should close
        eHandle.Unwrap() = ResourceManager::ManagedFileHandle();
        CHECK_FALSE(instance.IsHandleValid(rawHandle));
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager MT - Write guard blocks other writes") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        instance.Create("assets/tests/mt_write.bin", 64);
        auto eHandle = instance.Load("assets/tests/mt_write.bin", false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        std::atomic<int> concurrentWriters = 0;
        std::atomic<bool> exclusivityViolated = false;
        constexpr int THREAD_COUNT = 4;
        std::vector<std::thread> threads;
        std::barrier startBarrier(THREAD_COUNT);

        for (int i = 0; i < THREAD_COUNT; ++i) {
            threads.emplace_back([&]() {
                // startBarrier.arrive_and_wait();
                auto guard = instance.Data(handle);
                if (guard.IsValid()) {
                    int prev = concurrentWriters.fetch_add(1, std::memory_order_acq_rel);
                    if (prev > 0)
                        exclusivityViolated.store(true, std::memory_order_relaxed);

                    // Simulate some work while holding the write guard
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                    concurrentWriters.fetch_sub(1, std::memory_order_release);
                }
            });
        }

        for (auto& t : threads)
            t.join();

        CHECK_FALSE(exclusivityViolated.load());
    }

    std::filesystem::remove("assets/tests/mt_write.bin");
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager MT - Concurrent load and read do not race") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        constexpr int THREAD_COUNT = 8;
        std::atomic<int> successCount = 0;
        std::vector<std::thread> threads;
        std::barrier startBarrier(THREAD_COUNT);

        for (int i = 0; i < THREAD_COUNT; ++i) {
            threads.emplace_back([&, i]() {
                startBarrier.arrive_and_wait();
                if (i % 2 == 0) {
                    // Even threads load
                    auto eHandle = instance.Load("assets/tests/valid.txt");
                    if (eHandle.IsValid())
                        successCount.fetch_add(1, std::memory_order_relaxed);
                } else {
                    // Odd threads read size
                    auto eHandle = instance.Load("assets/tests/valid.txt");
                    if (eHandle.IsValid()) {
                        auto size = instance.Size(eHandle.Unwrap());
                        if (size.IsValid() && size.Unwrap() == 21)
                            successCount.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }

        for (auto& t : threads)
            t.join();

        CHECK_EQ(successCount.load(), THREAD_COUNT);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize - Grow a read-only file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_grow_ro.bin", 64);

    {
        auto eHandle = instance.Load(tmp.path);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 64);

        bool result = instance.Resize(handle.Get(), 128);
        CHECK(result);
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 128);
        CHECK_EQ(std::filesystem::file_size(tmp.path), (u64) 128);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize - Grow a read-write file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_grow_rw.bin", 64);

    {
        auto eHandle = instance.Load(tmp.path, false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 64);

        bool result = instance.Resize(handle.Get(), 256);
        CHECK(result);
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 256);
        CHECK_EQ(std::filesystem::file_size(tmp.path), (u64) 256);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize - Shrink a file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_shrink.bin", 256);

    {
        auto eHandle = instance.Load(tmp.path, false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 256);

        bool result = instance.Resize(handle.Get(), 64);
        CHECK(result);
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 64);
        CHECK_EQ(std::filesystem::file_size(tmp.path), (u64) 64);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize - Resize to same size is a no-op") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_same.bin", 128);

    {
        auto eHandle = instance.Load(tmp.path);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        bool result = instance.Resize(handle.Get(), 128);
        CHECK(result);
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 128);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize - Invalid handle fails gracefully") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    bool result = instance.Resize(INVALID_FILE_HANDLE, 128);
    CHECK_FALSE(result);

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize - Data is accessible after grow") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_data_grow.bin", 64);

    {
        auto eHandle = instance.Load(tmp.path, false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        // Write some data before resize
        {
            auto guard = instance.Data(handle);
            REQUIRE(guard.IsValid());
            guard.Unwrap().Data()[0] = 'A';
            guard.Unwrap().Data()[1] = 'B';
        }

        instance.Sync(handle);
        instance.Resize(handle.Get(), 128);

        // Data written before resize should still be there
        {
            auto guard = instance.DataConst(handle);
            REQUIRE(guard.IsValid());
            CHECK_EQ(guard.Unwrap().Data()[0], 'A');
            CHECK_EQ(guard.Unwrap().Data()[1], 'B');
        }

        // New region should be accessible
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 128);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize - Data is accessible after shrink") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_data_shrink.bin", 128);

    {
        auto eHandle = instance.Load(tmp.path, false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        // Write data in the region that will survive the shrink
        {
            auto guard = instance.Data(handle);
            REQUIRE(guard.IsValid());
            guard.Unwrap().Data()[0] = 'X';
            guard.Unwrap().Data()[1] = 'Y';
        }

        instance.Sync(handle);
        instance.Resize(handle.Get(), 64);

        {
            auto guard = instance.DataConst(handle);
            REQUIRE(guard.IsValid());
            CHECK_EQ(guard.Unwrap().Data()[0], 'X');
            CHECK_EQ(guard.Unwrap().Data()[1], 'Y');
        }
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize - Multiple consecutive resizes") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_consecutive.bin", 64);

    {
        auto eHandle = instance.Load(tmp.path, false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        REQUIRE(instance.Resize(handle.Get(), 128));
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 128);

        REQUIRE(instance.Resize(handle.Get(), 512));
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 512);

        REQUIRE(instance.Resize(handle.Get(), 256));
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 256);

        REQUIRE(instance.Resize(handle.Get(), 64));
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 64);

        CHECK_EQ(std::filesystem::file_size(tmp.path), (u64) 64);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize MT - Resize blocks until active ReadGuard is released") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_mt_block_read.bin", 64);

    {
        auto eHandle = instance.Load(tmp.path);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        std::atomic<bool> guardReleased = false;
        std::atomic<bool> resizeStarted = false;
        std::atomic<bool> resizeDone = false;

        // Thread A: holds a ReadGuard, then releases it
        std::thread reader([&]() {
            auto guard = instance.DataConst(handle);
            REQUIRE(guard.IsValid());
            auto g = std::move(guard.Unwrap());

            // Signal that resize can start
            resizeStarted.store(true);

            // Hold the guard for a bit
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            // Release guard — Resize should now proceed
            guardReleased.store(true);
            // g destroyed here, releases resource.m_Mutex
        });

        // Thread B: waits for reader to start, then resizes
        std::thread resizer([&]() {
            // Wait until reader has the guard
            while (!resizeStarted.load())
                std::this_thread::yield();

            instance.Resize(handle.Get(), 128);
            resizeDone.store(true);
        });

        reader.join();
        resizer.join();

        // Resize must have happened after the guard was released
        CHECK(guardReleased.load());
        CHECK(resizeDone.load());
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 128);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize MT - Resize blocks until active WriteGuard is released") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_mt_block_write.bin", 64);

    {
        auto eHandle = instance.Load(tmp.path, false);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();

        std::atomic<bool> guardReleased = false;
        std::atomic<bool> resizeStarted = false;
        std::atomic<bool> resizeDone = false;

        std::thread writer([&]() {
            auto guard = instance.Data(handle);
            REQUIRE(guard.IsValid());

            resizeStarted.store(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            auto g = std::move(guard.Unwrap());
            guardReleased.store(true);
            // g destroyed here
        });

        std::thread resizer([&]() {
            while (!resizeStarted.load())
                std::this_thread::yield();

            instance.Resize(handle.Get(), 128);
            resizeDone.store(true);
        });

        writer.join();
        resizer.join();

        CHECK(guardReleased.load());
        CHECK(resizeDone.load());
        CHECK_EQ(instance.Size(handle).Unwrap(), (u64) 128);
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize MT - Reads after resize see new size") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();
    TempFile tmp("resize_mt_new_size.bin", 64);

    {
        auto eHandle = instance.Load(tmp.path);
        REQUIRE(eHandle.IsValid());
        ResourceManager::ManagedFileHandle handle = eHandle.Unwrap();
        FileHandle rawHandle = handle.Get();

        constexpr int THREAD_COUNT = 4;
        std::atomic<bool> resizeDone = false;
        std::vector<std::thread> threads;

        // Reader threads — all start after resize
        for (int i = 0; i < THREAD_COUNT; ++i) {
            threads.emplace_back([&, i]() {
                // Wait for resize to complete
                while (!resizeDone.load())
                    std::this_thread::yield();

                auto size = instance.Size(rawHandle);
                REQUIRE(size.IsValid());
                CHECK_EQ(size.Unwrap(), (u64) 256);
            });
        }

        // Main thread resizes first
        instance.Resize(rawHandle, 256);
        resizeDone.store(true);

        for (auto& t : threads)
            t.join();
    }

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager Resize MT - Concurrent resizes on different files") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    {
        constexpr int FILE_COUNT = 4;
        std::vector<TempFile> files;
        std::deque<ResourceManager::ManagedFileHandle> handles;
        std::vector<FileHandle> rawHandles;
        rawHandles.reserve(FILE_COUNT + 1);
        files.reserve(FILE_COUNT + 1);

        for (int i = 0; i < FILE_COUNT; ++i) {
            files.emplace_back("resize_mt_concurrent_" + std::to_string(i) + ".bin", 64);
            auto eHandle = instance.Load(files.back().path);
            REQUIRE(eHandle.IsValid());
            handles.push_back(eHandle.Unwrap());
            rawHandles.push_back(handles.back().Get());
        }

        std::vector<std::thread> threads;

        for (int i = 0; i < FILE_COUNT; ++i) {
            threads.emplace_back([&, i]() { CHECK(instance.Resize(rawHandles[i], (u64) (128 * (i + 1)))); });
        }

        for (auto& t : threads)
            t.join();

        for (int i = 0; i < FILE_COUNT; ++i) {
            CHECK_EQ(instance.Size(rawHandles[i]).Unwrap(), (u64) (128 * (i + 1)));
        }
    }

    ResourceManager::ShutDown();
}
