#pragma once

#include "axpch.hpp"

#include "../Core.hpp"
#include "spdlog/spdlog.h"

namespace Axle {
	class AXLE_API Log {
	public:
		/**
		* Initializes the logger and its singletons
		*
		* Important: This has to be called before using the macros
		* 
		* It is safe to call multiple times, it simply displays a warning after the first call.
		*/
		static void Init();

		/**
		* Gets the core logger singleton
		* This function should only be used by the macro
		* 
		* @returns Returns a reference to the Core logger
		*/
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() {
			return s_CoreLogger;
		}
		
		/**
		* Gets the client logger singleton
		* This function should only be used by the macro
		*
		* @returns Returns a reference to the client logger
		*/
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {
			return s_ClientLogger;
		}

	private:
		/// Core logger singleton
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		/// Client logger singleton
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define AX_CORE_TRACE(...) ::Axle::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define AX_CORE_INFO(...) ::Axle::Log::GetCoreLogger()->info(__VA_ARGS__)
#define AX_CORE_WARN(...) ::Axle::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define AX_CORE_ERROR(...) ::Axle::Log::GetCoreLogger()->error(__VA_ARGS__)
#define AX_CORE_CRITICAL(...) ::Axle::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define AX_TRACE(...) ::Axle::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AX_INFO(...) ::Axle::Log::GetClientLogger()->info(__VA_ARGS__)
#define AX_WARN(...) ::Axle::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AX_ERROR(...) ::Axle::Log::GetClientLogger()->error(__VA_ARGS__)
#define AX_CRITICAL(...) ::Axle::Log::GetClientLogger()->critical(__VA_ARGS__)
