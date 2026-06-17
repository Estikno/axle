#pragma once

#include "axpch.hpp"

#include "../Core.hpp"
#include "../Types.hpp"
#include "spdlog/spdlog.h"

namespace Axle {
    /// Defines how verbose should the logger be. Each higher level includes all the previous ones
    enum class LogVerbosity { Critical = 0, Error, Warn, Info, All };

    /// Contains a list to all avaiblable channels.
    enum class LogChannels { Core = 0, Events, Input, Resources, Window };

    class AXLE_API Log {
    public:
        Log() {};
        ~Log() {};

        /**
         * Initializes the logger and its singletons
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         *
         * Important: This has to be called before using the macros
         *
         * It is safe to call multiple times, it simply displays a warning after the first call.
         */
        static void Init();

        /**
         * Shutdowns the manager, important to call when no other component depends on it anymore
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         */
        static void ShutDown();

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

        /// Controls how verbose is the logger
        LogVerbosity m_Verbosity = LogVerbosity::All;

        /// Marks channels are enabled
        /// TODO: Make it so that there might be more than 64 channels
        std::atomic<u64> m_EnabledChannels = std::numeric_limits<u64>::max();
    };
} // namespace Axle

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
