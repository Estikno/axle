#include "Panic.hpp"

#include "axpch.hpp"

#include <cstdio>
#include <cstdlib>

namespace Axle::Error {
	[[noreturn]] void PanicImpl(const char* s) noexcept {
		std::fputs(s, stderr);
		std::abort();
	}
}