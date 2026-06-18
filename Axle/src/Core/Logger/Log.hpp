#pragma once

#include "axpch.hpp"

#include "../Core.hpp"
#include "../Types.hpp"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace Axle {
    /// Defines how verbose should the logger be. Each higher level includes all the previous ones
    enum class LogVerbosity { Critical = 0, Error, Warn, Info, All };

    /// Contains a list to all avaiblable channels.
    enum class LogChannel { Core = 0, Events, Input, Resources, Window, MaxChannels };

    /// Human-readable names — index must match LogChannel order
    inline constexpr std::string_view CHANNEL_NAMES[] = {"Core", "Events", "Input", "Resources", "Window"};

    inline constexpr std::string_view ChannelName(LogChannel ch) {
        return CHANNEL_NAMES[static_cast<u8>(ch)];
    }

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
         * Gets the Logger singleton
         *
         * The manager has to have already been initilized before getting the instance.
         *
         * @returns Returns a reference to the Event Handler
         */
        inline static Log& GetInstance() noexcept {
            return *s_Instance;
        }

        /**
         * Gets the core logger singleton
         * This function should only be used by the macro
         *
         * @returns Returns a reference to the Core logger
         */
        inline std::shared_ptr<spdlog::logger>& GetCoreLogger(LogChannel channel) {
            return m_ChannelLoggers[static_cast<u8>(channel)];
        }

        /**
         * Gets the client logger singleton
         * This function should only be used by the macro
         *
         * @returns Returns a reference to the client logger
         */
        inline std::shared_ptr<spdlog::logger>& GetClientLogger(LogChannel channel) {
            return m_ChannelLoggers[static_cast<u8>(channel)];
        }

        void SetVerbosity(LogVerbosity verbosity) {
            for (u8 i = 0; i < static_cast<u8>(LogChannel::MaxChannels); ++i) {
                m_ChannelLoggers[i]->set_level(ToSpdlogLevel(verbosity));
            }
        }

        void EnableChannel(LogChannel channel) {
            m_ChannelLoggers[static_cast<u8>(channel)]->set_level(spdlog::level::trace);
        }

        void DisableChannels(LogChannel channel) {
            m_ChannelLoggers[static_cast<u8>(channel)]->set_level(spdlog::level::off);
        }

    private:
        /// Maps LogVerbosity to spdlog level
        inline spdlog::level::level_enum ToSpdlogLevel(LogVerbosity v) {
            switch (v) {
                case LogVerbosity::Critical:
                    return spdlog::level::critical;
                case LogVerbosity::Error:
                    return spdlog::level::err;
                case LogVerbosity::Warn:
                    return spdlog::level::warn;
                case LogVerbosity::Info:
                    return spdlog::level::info;
                case LogVerbosity::All:
                    return spdlog::level::trace;
            }
            return spdlog::level::trace;
        }

        static std::unique_ptr<Log> s_Instance;

        // Sinks
        std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_ConsoleSink;

        // Loggers
        std::array<std::shared_ptr<spdlog::logger>, static_cast<u8>(LogChannel::MaxChannels)> m_ChannelLoggers;

        /// Core logger singleton
        // std::shared_ptr<spdlog::logger> m_CoreLogger;
        /// Client logger singleton
        // std::shared_ptr<spdlog::logger> m_ClientLogger;

        /// Marks channels are enabled
        /// TODO: Make it so that there might be more than 64 channels
        std::atomic<u64> m_EnabledChannels = std::numeric_limits<u64>::max();
    };
} // namespace Axle

// Core log macros
#define AX_CORE_TRACE(channel, ...) ::Axle::Log::GetInstance().GetCoreLogger(channel)->trace(__VA_ARGS__)
#define AX_CORE_INFO(channel, ...) ::Axle::Log::GetInstance().GetCoreLogger(channel)->info(__VA_ARGS__)
#define AX_CORE_WARN(channel, ...) ::Axle::Log::GetInstance().GetCoreLogger(channel)->warn(__VA_ARGS__)
#define AX_CORE_ERROR(channel, ...) ::Axle::Log::GetInstance().GetCoreLogger(channel)->error(__VA_ARGS__)
#define AX_CORE_CRITICAL(channel, ...) ::Axle::Log::GetInstance().GetCoreLogger(channel)->critical(__VA_ARGS__)

// Client log macros
#define AX_TRACE(channel, ...) ::Axle::Log::GetInstance().GetClientLogger(channel)->trace(__VA_ARGS__)
#define AX_INFO(channel, ...) ::Axle::Log::GetInstance().GetClientLogger(channel)->info(__VA_ARGS__)
#define AX_WARN(channel, ...) ::Axle::Log::GetInstance().GetClientLogger(channel)->warn(__VA_ARGS__)
#define AX_ERROR(channel, ...) ::Axle::Log::GetInstance().GetClientLogger(channel)->error(__VA_ARGS__)
#define AX_CRITICAL(channel, ...) ::Axle::Log::GetInstance().GetClientLogger(channel)->critical(__VA_ARGS__)
