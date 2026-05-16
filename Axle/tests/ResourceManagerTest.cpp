#include <doctest.h>

#include "Core/Types.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include <filesystem>

using namespace Axle;

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

    Expected<FileHandle> eHandle = instance.Load("assets/tests/valid.txt");
    REQUIRE(eHandle.IsValid());
    FileHandle handle = eHandle.Unwrap();

    CHECK(instance.AvailableIndexes().empty());
    CHECK_EQ(instance.LargestAvailableIndex(), (u16) 1);
    CHECK_EQ(instance.MagicNumberCounter(), (u16) 1);

    CHECK_EQ(instance.GetIndexFromHandle(handle), (u16) 0);
    CHECK_EQ(instance.GetMagicFromHandle(handle), (u16) 0);

    REQUIRE_EQ(instance.Size(handle).Unwrap(), (u64) 21);

    const char* p = instance.DataConst(handle).Unwrap();

    CHECK(p[0] == 'T');
    CHECK(p[1] == 'h');
    CHECK(p[2] == 'i');
    CHECK(p[3] == 's');
    CHECK(p[4] == ' ');

    REQUIRE(instance.Close(handle));

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load non-existing file fails") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<FileHandle> eHandle = instance.Load("assets/tests/novalid.txt");

    CHECK(!eHandle.IsValid());

    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load same file twice returns same handle") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<FileHandle> eHandle1 = instance.Load("assets/tests/valid.txt");
    Expected<FileHandle> eHandle2 = instance.Load("assets/tests/valid.txt");

    REQUIRE(eHandle1.IsValid());
    REQUIRE(eHandle2.IsValid());

    // Same file — should return the same handle, no new index allocated
    CHECK_EQ(eHandle1.Unwrap(), eHandle2.Unwrap());
    CHECK_EQ(instance.LargestAvailableIndex(), (u16) 1);
    CHECK_EQ(instance.MagicNumberCounter(), (u16) 1);

    instance.Close(eHandle1.Unwrap());
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load multiple different files") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<FileHandle> eHandle1 = instance.Load("assets/tests/valid.txt");
    Expected<FileHandle> eHandle2 = instance.Load("assets/tests/valid2.bin");

    REQUIRE(eHandle1.IsValid());
    REQUIRE(eHandle2.IsValid());

    FileHandle h1 = eHandle1.Unwrap();
    FileHandle h2 = eHandle2.Unwrap();

    // Different handles
    CHECK_NE(h1, h2);
    CHECK_EQ(instance.LargestAvailableIndex(), (u16) 2);
    CHECK_EQ(instance.MagicNumberCounter(), (u16) 2);

    // Different indexes
    CHECK_EQ(instance.GetIndexFromHandle(h1), (u16) 0);
    CHECK_EQ(instance.GetIndexFromHandle(h2), (u16) 1);

    instance.Close(h1);
    instance.Close(h2);
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Load read-write file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<FileHandle> eHandle = instance.Load("assets/tests/valid.txt", false);
    REQUIRE(eHandle.IsValid());
    FileHandle handle = eHandle.Unwrap();

    // Data() should succeed for a read-write file
    Expected<char*> data = instance.Data(handle);
    CHECK(data.IsValid());

    instance.Close(handle);
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Data() fails on read-only file") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<FileHandle> eHandle = instance.Load("assets/tests/valid.txt", true);
    REQUIRE(eHandle.IsValid());
    FileHandle handle = eHandle.Unwrap();

    // Data() should fail since file is read-only
    Expected<char*> data = instance.Data(handle);
    CHECK_FALSE(data.IsValid());

    // DataConst() should succeed
    Expected<const char*> dataConst = instance.DataConst(handle);
    CHECK(dataConst.IsValid());

    instance.Close(handle);
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Close releases index for reuse") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<FileHandle> eHandle1 = instance.Load("assets/tests/valid.txt");
    REQUIRE(eHandle1.IsValid());
    FileHandle h1 = eHandle1.Unwrap();
    CHECK_EQ(instance.GetIndexFromHandle(h1), (u16) 0);

    instance.Close(h1);

    // Index 0 should now be available again
    CHECK_FALSE(instance.AvailableIndexes().empty());

    // Loading a new file should reuse index 0
    Expected<FileHandle> eHandle2 = instance.Load("assets/tests/valid2.bin", false);
    REQUIRE(eHandle2.IsValid());
    FileHandle h2 = eHandle2.Unwrap();
    CHECK_EQ(instance.GetIndexFromHandle(h2), (u16) 0);

    // Magic should have incremented though — stale handle h1 is now invalid
    CHECK_NE(instance.GetMagicFromHandle(h1), instance.GetMagicFromHandle(h2));

    CHECK_EQ(instance.LargestAvailableIndex(), (u16) 1);
    CHECK_EQ(instance.MagicNumberCounter(), (u16) 2);

    instance.Close(h2);
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Stale handle is rejected after close") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<FileHandle> eHandle = instance.Load("assets/tests/valid.txt");
    REQUIRE(eHandle.IsValid());
    FileHandle handle = eHandle.Unwrap();
    instance.Close(handle);

    // Reopen the file — new magic, old handle is stale
    Expected<FileHandle> eHandle2 = instance.Load("assets/tests/valid.txt");
    REQUIRE(eHandle2.IsValid());

    // Size() with stale handle should fail magic check and return invalid
    Expected<u64> size = instance.Size(handle);
    CHECK_FALSE(size.IsValid());

    instance.Close(eHandle2.Unwrap());
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

    Expected<FileHandle> eHandle = instance.Load("assets/tests/valid.txt", true);
    REQUIRE(eHandle.IsValid());
    FileHandle handle = eHandle.Unwrap();

    // Syncing a read-only map is a no-op and returns false
    CHECK_FALSE(instance.Sync(handle));

    instance.Close(handle);
    ResourceManager::ShutDown();
}

TEST_CASE("ResourceManager - Sync read-write file succeeds") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    Expected<FileHandle> eHandle = instance.Load("assets/tests/valid.txt", false);
    REQUIRE(eHandle.IsValid());
    FileHandle handle = eHandle.Unwrap();

    CHECK(instance.Sync(handle));

    instance.Close(handle);
    ResourceManager::ShutDown();
}

// ─────────────────────────────────────────────
//  ShutDown
// ─────────────────────────────────────────────

TEST_CASE("ResourceManager - ShutDown closes all open files") {
    ResourceManager::Init();
    ResourceManager& instance = ResourceManager::GetInstance();

    instance.Load("assets/tests/valid.txt");
    CHECK_EQ(instance.LargestAvailableIndex(), (u16) 1);

    // ShutDown should close everything without crashing
    ResourceManager::ShutDown();

    // Re-init to verify clean state
    ResourceManager::Init();
    ResourceManager& instance2 = ResourceManager::GetInstance();
    CHECK_EQ(instance2.LargestAvailableIndex(), (u16) 0);
    CHECK_EQ(instance2.MagicNumberCounter(), (u16) 0);
    ResourceManager::ShutDown();
}
