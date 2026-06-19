#include <doctest.h>

#include "Core/Logger/Log.hpp"

TEST_CASE("Logger initializes correctly") {
    Axle::Log::Init();

    // If the core logger works we assume everything is ok as all loggers are threaded equally
    auto coreLogger = Axle::Log::GetInstance().GetCoreLogger(Axle::LogChannel::Core);
    auto clientLogger = Axle::Log::GetInstance().GetClientLogger();

    CHECK(coreLogger != nullptr);
    CHECK(clientLogger != nullptr);

    CHECK_NOTHROW(AX_CORE_INFO(Axle::LogChannel::Core, "Core logger tested"));
    CHECK_NOTHROW(AX_TRACE("Client logger tested"));
}
