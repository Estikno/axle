#pragma once

#ifdef AX_PLATFORM_WINDOWS
	#ifdef AX_BUILD_DLL
		#define AXLE_API __declspec(dllexport)
	#else
		#define AXLE_API __declspec(dllimport)
	#endif // AX_BUILD_DLL
#else
	#error Axle only supports windows
#endif // AX_PLATFORM_WINDOWS

