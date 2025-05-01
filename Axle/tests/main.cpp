#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "Core/Logger/Log.hpp"

TEST_CASE("Logger initializes correctly", "[logger]") {
	Axle::Log::Init();

	auto coreLogger = Axle::Log::GetCoreLogger();
	auto clientLogger = Axle::Log::GetClientLogger();

	CHECK(coreLogger != nullptr);
	CHECK(clientLogger != nullptr);

	CHECK_NOTHROW(AX_CORE_INFO("Core logger tested"));
	CHECK_NOTHROW(AX_TRACE("Client logger tested"));
}
