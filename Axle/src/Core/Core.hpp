#pragma once

// Simplifies imports and exports to dll with a macro
#ifdef AX_PLATFORM_WINDOWS
#    ifdef AX_BUILD_DLL
#        define AXLE_API __declspec(dllexport)
#    else
#        define AXLE_API __declspec(dllimport)
#    endif
#elif AX_PLATFORM_LINUX
#    ifdef AX_BUILD_DLL
#        define AXLE_API __attribute__((visibility("default")))
#    else
#        define AXLE_API
#    endif
#else
#    error Axle only supports windows and linux
#endif

// ── Architecture detection ────────────────────────────────────────────────────
#if defined(__x86_64__) || defined(_M_X64)
#    define AX_ARCH_X64
#elif defined(__i386__) || defined(_M_IX86)
#    define AX_ARCH_X86
#    error Axle does not support x86 (32-bit)
#elif defined(__aarch64__) || defined(_M_ARM64)
#    define AX_ARCH_ARM64
#    error Axle does not support ARM64
#elif defined(__arm__) || defined(_M_ARM)
#    define AX_ARCH_ARM
#    error Axle does not support ARM (32-bit)
#else
#    error Axle only supports x64
#endif

// Only exports if it's for testing
#ifdef AXLE_TESTING
#    define AXLE_TEST_API AXLE_API
#else
#    define AXLE_TEST_API
#endif

#define AX_BIND_EVENT_FN(fn) \
    [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
