#pragma once

#include "Core/Types.hpp"

namespace Axle {
    const char* GLErrorToString(u32 err);

    // Drains ALL pending errors (glGetError only returns one at a time)
    // and logs each with the call site that triggered the check.
    bool CheckGLErrors(const char* expr, const char* file, int line);
} // namespace Axle

// Automates error checking
#ifdef AX_DEBUG
#    define AX_GL_CALL(x)                                  \
        do {                                               \
            x;                                             \
            ::Axle::CheckGLErrors(#x, __FILE__, __LINE__); \
        } while (0);
#else
#    define AX_GL_CALL(x) x
#endif // AX_DEBUG
