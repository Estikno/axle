#pragma once

// Simplifies imports and exports to dll with a macro
#ifdef AX_PLATFORM_WINDOWS
#ifdef AX_BUILD_DLL
#define AXLE_API __declspec(dllexport)
#else
#define AXLE_API __declspec(dllimport)
#endif // AX_BUILD_DLL
#elif AX_PLATFORM_LINUX
#ifdef AX_BUILD_DLL
#define AXLE_API __attribute__((visibility("default")))
#else
#define AXLE_API
#endif
#else
#error Axle only supports windows
#endif // AX_PLATFORM_WINDOWS
