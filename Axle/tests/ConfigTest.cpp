#include <doctest.h>

#include "Core/Config/Config.hpp"
#include "Core/Logger/Log.hpp"

#include <filesystem>
#include <fstream>

using namespace Axle;

// Helpers — write a temp INI file and clean it up per test
static const std::string k_TestFile = "assets/tests/test_config.ini";

static void WriteTestIni(const std::string& content) {
    std::ofstream f(k_TestFile);
    f << content;
}

static void Cleanup() {
    std::filesystem::remove(k_TestFile);
}

// RAII guard so tests always clean up even on failure
struct ConfigGuard {
    ConfigGuard(const std::string& content) {
        WriteTestIni(content);
        Axle::Log::Init();
        Axle::Config::Init(k_TestFile);
    }
    ~ConfigGuard() {
        Axle::Config::ShutDown();
        Cleanup();
    }
};

// ─────────────────────────────────────────────────────────────────────────────
TEST_SUITE("Config") {
    TEST_CASE("Init loads existing file") {
        ConfigGuard g("[Window]\nwidth = 1280\n");
        auto val = Axle::Config::Get<Axle::i32>("Window", "width");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == 1280);
    }

    TEST_CASE("Get returns error for missing section") {
        ConfigGuard g("[Window]\nwidth = 1280\n");
        auto val = Axle::Config::Get<Axle::i32>("Renderer", "vsync");
        CHECK(!val.IsValid());
    }

    TEST_CASE("Get returns error for missing key") {
        ConfigGuard g("[Window]\nwidth = 1280\n");
        auto val = Axle::Config::Get<Axle::i32>("Window", "height");
        CHECK(!val.IsValid());
    }

    TEST_CASE("Set creates new key and Get retrieves it") {
        ConfigGuard g("[Window]\n");
        CHECK(Axle::Config::Set<Axle::i32>("Window", "width", 1920));
        auto val = Axle::Config::Get<Axle::i32>("Window", "width");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == 1920);
    }

    TEST_CASE("Set overwrites existing key") {
        ConfigGuard g("[Window]\nwidth = 1280\n");
        CHECK(Axle::Config::Set<Axle::i32>("Window", "width", 1920));
        auto val = Axle::Config::Get<Axle::i32>("Window", "width");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == 1920);
    }

    TEST_CASE("Bool round-trip") {
        ConfigGuard g("[Renderer]\nvsync = true\n");
        auto val = Axle::Config::Get<bool>("Renderer", "vsync");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == true);

        CHECK(Axle::Config::Set<bool>("Renderer", "vsync", false));
        auto val2 = Axle::Config::Get<bool>("Renderer", "vsync");
        REQUIRE(val2.IsValid());
        CHECK(val2.Unwrap() == false);
    }

    TEST_CASE("Float round-trip") {
        ConfigGuard g("[Renderer]\ngamma = 2.2\n");
        auto val = Axle::Config::Get<Axle::f32>("Renderer", "gamma");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == doctest::Approx(2.2f).epsilon(0.001f));
    }

    TEST_CASE("String round-trip") {
        ConfigGuard g("[Window]\ntitle = Axle Engine\n");
        auto val = Axle::Config::Get<std::string>("Window", "title");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == "Axle Engine");
    }

    TEST_CASE("DeleteName removes a key") {
        ConfigGuard g("[Window]\nwidth = 1280\nheight = 720\n");
        CHECK(Axle::Config::DeleteName("Window", "width"));
        CHECK(!Axle::Config::Get<Axle::i32>("Window", "width").IsValid());
        CHECK(Axle::Config::Get<Axle::i32>("Window", "height").IsValid());
    }

    TEST_CASE("DeleteSection removes all keys") {
        ConfigGuard g("[Window]\nwidth = 1280\nheight = 720\n");
        CHECK(Axle::Config::DeleteSection("Window"));
        CHECK(!Axle::Config::Get<Axle::i32>("Window", "width").IsValid());
        CHECK(!Axle::Config::Get<Axle::i32>("Window", "height").IsValid());
    }

    TEST_CASE("Save persists changes to disk") {
        WriteTestIni("[Window]\nwidth = 1280\n");
        Axle::Config::Init(k_TestFile);
        Axle::Config::Set<Axle::i32>("Window", "width", 1920);
        Axle::Config::Save();
        Axle::Config::ShutDown();

        // Re-load and verify
        Axle::Config::Init(k_TestFile);
        auto val = Axle::Config::Get<Axle::i32>("Window", "width");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == 1920);
        Axle::Config::ShutDown();
        Cleanup();
    }

    TEST_CASE("Double Init is ignored") {
        ConfigGuard g("[Window]\nwidth = 1280\n");
        // Should warn but not crash or reset state
        Axle::Config::Init(k_TestFile);
        auto val = Axle::Config::Get<Axle::i32>("Window", "width");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == 1280);
    }
}

TEST_SUITE("Config.GetOrSet") {
    TEST_CASE("Returns existing value without modifying it") {
        ConfigGuard g("[Window]\nwidth = 1280\n");

        i32 width = Axle::Config::GetOrSet<i32>("Window", "width", 1920);
        CHECK(width == 1280);

        auto raw = Axle::Config::Get<i32>("Window", "width");
        REQUIRE(raw.IsValid());
        CHECK(raw.Unwrap() == 1280);
    }

    TEST_CASE("Returns default when key is missing") {
        ConfigGuard g("[Window]\n");

        i32 width = Axle::Config::GetOrSet<i32>("Window", "width", 1920);
        CHECK(width == 1920);
    }

    TEST_CASE("Writes default back to config when key is missing") {
        ConfigGuard g("[Window]\n");

        Axle::Config::GetOrSet<i32>("Window", "width", 1920);

        auto val = Axle::Config::Get<i32>("Window", "width");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == 1920);
    }

    TEST_CASE("Returns default when section is missing entirely") {
        ConfigGuard g("");

        bool vsync = Axle::Config::GetOrSet<bool>("Renderer", "vsync", true);
        CHECK(vsync == true);

        auto val = Axle::Config::Get<bool>("Renderer", "vsync");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == true);
    }

    TEST_CASE("Persists default to disk after Save") {
        WriteTestIni("[Window]\n");
        Axle::Config::Init(k_TestFile);

        Axle::Config::GetOrSet<i32>("Window", "width", 1920);
        Axle::Config::Save();
        Axle::Config::ShutDown();

        Axle::Config::Init(k_TestFile);
        auto val = Axle::Config::Get<i32>("Window", "width");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == 1920);
        Axle::Config::ShutDown();
        Cleanup();
    }

    TEST_CASE("Works for float type") {
        ConfigGuard g("[Renderer]\n");
        f32 gamma = Axle::Config::GetOrSet<f32>("Renderer", "gamma", 2.2f);
        CHECK(gamma == doctest::Approx(2.2f).epsilon(0.001f));
    }

    TEST_CASE("Works for bool type") {
        ConfigGuard g("[Renderer]\n");
        bool vsync = Axle::Config::GetOrSet<bool>("Renderer", "vsync", false);
        CHECK(vsync == false);
    }

    TEST_CASE("Works for string type") {
        ConfigGuard g("[Window]\n");
        std::string title = Axle::Config::GetOrSet<std::string>("Window", "title", std::string("Axle Engine"));
        CHECK(title == "Axle Engine");
    }

    TEST_CASE("Concurrent GetOrSet on the same missing key: exactly one default wins") {
        ConfigGuard g("[Window]\n");

        constexpr int kThreads = 16;
        std::vector<std::thread> threads;
        std::vector<i32> results(kThreads);

        for (int i = 0; i < kThreads; ++i) {
            threads.emplace_back([&, i]() { results[i] = Axle::Config::GetOrSet<i32>("Window", "width", 1000 + i); });
        }
        for (auto& t : threads)
            t.join();

        // Every thread must observe the SAME final value — whichever thread's
        // write actually landed first under the lock — never a mix of values.
        i32 finalValue = results[0];
        for (i32 r : results)
            CHECK(r == finalValue);

        auto val = Axle::Config::Get<i32>("Window", "width");
        REQUIRE(val.IsValid());
        CHECK(val.Unwrap() == finalValue);
    }
}
