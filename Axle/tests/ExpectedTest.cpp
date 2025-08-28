#include <doctest.h>

#include "Core/Logger/Log.hpp"
#include "Other/CustomTypes/Expected.hpp"

using namespace Axle;

TEST_CASE("Expected basic value") {
    auto e = Expected<int>::FromCode([]() { return 42; });

    CHECK(e.IsValid() == true);
    CHECK(e.Unwrap() == 42);
}

TEST_CASE("Expected basic exception") {
    auto e = Expected<int>::FromException(std::make_exception_ptr(std::runtime_error("fail")));

    CHECK_FALSE(e.IsValid());

    bool caught = false;
    try {
        e.Unwrap();
    } catch (const std::runtime_error& ex) {
        caught = true;
        CHECK(std::string(ex.what()) == "fail");
    }
    CHECK(caught == true);
}

TEST_CASE("Expected copy and move constructors only") {
    auto e1 = Expected<std::string>::FromCode([]() { return "hola"; });
    CHECK(e1.IsValid());
    CHECK(e1.Unwrap() == "hola");

    // Copy constructor
    Expected<std::string> e2(e1);
    CHECK(e2.IsValid());
    CHECK(e2.Unwrap() == "hola");

    // Move constructor
    Expected<std::string> e3(std::move(e1));
    CHECK(e3.IsValid());
    CHECK(e3.Unwrap() == "hola");
}

TEST_CASE("Expected swap") {
    auto ok = Expected<int>::FromCode([]() { return 123; });
    auto err = Expected<int>::FromException(std::make_exception_ptr(std::runtime_error("boom")));

    ok.Swap(err);

    CHECK_FALSE(ok.IsValid());
    CHECK(err.IsValid() == true);
    CHECK(err.Unwrap() == 123);
}

TEST_CASE("Expected ThrowIfException passthrough") {
    auto ok = Expected<int>::FromCode([]() { return 7; });
    CHECK_NOTHROW(ok.Unwrap());

    auto bad = Expected<int>::FromException(std::make_exception_ptr(std::runtime_error("bad")));
    CHECK_THROWS_AS(bad.Unwrap(), std::runtime_error);
}
