#pragma once

#include <memory>

#include "Core.hpp"
#include "spdlog/spdlog.h"

namespace Axle {
	class AXLE_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define AX_CORE_TRACE(...)      ::Axle::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define AX_CORE_INFO(...)       ::Axle::Log::GetCoreLogger()->info(__VA_ARGS__)
#define AX_CORE_WARN(...)       ::Axle::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define AX_CORE_ERROR(...)      ::Axle::Log::GetCoreLogger()->error(__VA_ARGS__)
#define AX_CORE_CRITICAL(...)   ::Axle::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Core log macros
#define AX_TRACE(...)           ::Axle::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AX_INFO(...)            ::Axle::Log::GetClientLogger()->info(__VA_ARGS__)
#define AX_WARN(...)            ::Axle::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AX_ERROR(...)           ::Axle::Log::GetClientLogger()->error(__VA_ARGS__)
#define AX_CRITICAL(...)        ::Axle::Log::GetClientLogger()->critical(__VA_ARGS__)
