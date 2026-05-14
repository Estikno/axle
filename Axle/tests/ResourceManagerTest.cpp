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
