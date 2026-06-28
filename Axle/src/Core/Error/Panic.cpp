#include "Panic.hpp"

#include "axpch.hpp"

#include "Core/Systems.hpp"

#include <cstdio>
#include <cstdlib>

namespace Axle::Error {
    [[noreturn]] void PanicImpl(const char* s) noexcept {
        std::fputs(s, stderr);

        // Deletes all systems nicely
        // NOTE: We can't safely shut down the job system by now becase we dont know which thread is able to call
        // this
        ShutdownSystems();

        std::abort();
    }
} // namespace Axle::Error
