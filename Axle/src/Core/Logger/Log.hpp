#pragma once

#include "axpch.hpp"

#include "../Core.hpp"
#include "../Types.hpp"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace Axle {
    /// Defines how verbose should the logger be. Each higher level includes all the previous ones
    enum class LogVerbosity { Critical = 0, Error, Warn, Info, All, MaxVerbosities };

    inline constexpr std::string_view VERBOSITY_NAMES[] = {"Critical", "Error", "Warn", "Info", "All"};

    /// Contains a list to all avaiblable channels.
    enum class LogChannel {
        Core = 0,
        Client,
        Config,
        Events,
        Input,
        Resources,
        Window,
        Debug,
        Renderer,
        Other,
        MaxChannels
    };

    /// Human-readable names — index must match LogChannel order
    inline constexpr std::string_view CHANNEL_NAMES[] =
        {"Core", "Client", "Config", "Events", "Input", "Resources", "Window", "Debug", "Renderer", "Other"};

    inline constexpr std::string_view ChannelName(LogChannel ch) {
        return CHANNEL_NAMES[static_cast<u8>(ch)];
    }

    inline LogVerbosity VerbosityFromStr(const std::string& name) {
        auto it = std::find(std::begin(VERBOSITY_NAMES), std::end(VERBOSITY_NAMES), name);
        if (it != std::end(VERBOSITY_NAMES))
            return static_cast<LogVerbosity>(std::distance(std::begin(VERBOSITY_NAMES), it));

        return LogVerbosity::All;
    }

    inline LogChannel ChannelFromStr(const std::string& name) {
        auto it = std::find(std::begin(CHANNEL_NAMES), std::end(CHANNEL_NAMES), name);
        if (it != std::end(CHANNEL_NAMES))
            return static_cast<LogChannel>(std::distance(std::begin(CHANNEL_NAMES), it));

        return LogChannel::Core;
    }

    class AXLE_API Log {
    public:
        Log() {};
        ~Log() {};

        Log(const Log&) = delete;
        Log& operator=(const Log&) = delete;

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
         *
         * Thread safe
         */
        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger(LogChannel channel) {
            return s_Instance->GetCoreLoggerImpl(channel);
        }

        /**
         * Gets the client logger singleton
         * This function should only be used by the macro
         *
         * @returns Returns a reference to the client logger
         *
         * Thread safe
         */
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {
            return s_Instance->GetClientLoggerImpl();
        }

        /**
         * Sets the global verbosity
         *
         * @param verbosity The desired verbosity
         *
         * Thread safe
         * */
        inline static void SetVerbosity(LogVerbosity verbosity) {
            return s_Instance->SetVerbosityImpl(verbosity);
        }

        /**
         * Gets the current verbosity.
         *
         * @returns The logger verbosity
         *
         * Thread safe
         * */
        inline static LogVerbosity GetCurrentVerbosity() {
            return s_Instance->GetCurrentVerbosityImpl();
        }

        /**
         * Enables the specified channel
         *
         * @param channel The channel to enable
         *
         * Thread safe
         * */
        inline static void EnableChannel(LogChannel channel) {
            s_Instance->EnableChannelImpl(channel);
        }

        /**
         * Disables the specified channel
         *
         * @param channel The channel to disable
         *
         * Thread safe
         * */
        inline static void DisableChannel(LogChannel channel) {
            s_Instance->DisableChannelImpl(channel);
        }

        /**
         * Checks whether the specified channel is enabled or not
         *
         * @param channel The channel to check
         *
         * @returns true if the channel is enabled, false otherwise
         *
         * Thread safe
         * */
        inline static bool IsChannelEnabled(LogChannel channel) {
            return s_Instance->IsChannelEnabledImpl(channel);
        }

    private:
        // Static methods implementations
        inline std::shared_ptr<spdlog::logger>& GetCoreLoggerImpl(LogChannel channel) {
            return m_ChannelLoggers[static_cast<u8>(channel)];
        }
        inline std::shared_ptr<spdlog::logger>& GetClientLoggerImpl() {
            return m_ChannelLoggers[static_cast<u8>(LogChannel::Client)];
        }
        void SetVerbosityImpl(LogVerbosity verbosity);
        LogVerbosity GetCurrentVerbosityImpl() const {
            return m_Verbosity.load(std::memory_order_acquire);
        }
        inline void EnableChannelImpl(LogChannel channel) {
            // Set channel to current verbosity
            m_ChannelLoggers[static_cast<u8>(channel)]->set_level(
                ToSpdlogLevel(m_Verbosity.load(std::memory_order_acquire)));
        }
        inline void DisableChannelImpl(LogChannel channel) {
            m_ChannelLoggers[static_cast<u8>(channel)]->set_level(spdlog::level::off);
        }
        inline bool IsChannelEnabledImpl(LogChannel channel) {
            return m_ChannelLoggers[static_cast<u8>(channel)]->level() != spdlog::level::off;
        }

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
                case LogVerbosity::MaxVerbosities:
                    return spdlog::level::trace;
            }
            return spdlog::level::trace;
        }

        static std::unique_ptr<Log> s_Instance;

        // Sinks
        std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_ConsoleSink;
        std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> m_FileSink;

        // Loggers
        std::array<std::shared_ptr<spdlog::logger>, static_cast<u8>(LogChannel::MaxChannels)> m_ChannelLoggers;

        std::atomic<LogVerbosity> m_Verbosity{LogVerbosity::All};
        static_assert(std::atomic<LogVerbosity>::is_always_lock_free, "LogVerbosity is not lock free");
    };
} // namespace Axle

// Core log macros
#define AX_CORE_TRACE(channel, ...) ::Axle::Log::GetCoreLogger(channel)->trace(__VA_ARGS__)
#define AX_CORE_INFO(channel, ...) ::Axle::Log::GetCoreLogger(channel)->info(__VA_ARGS__)
#define AX_CORE_WARN(channel, ...) ::Axle::Log::GetCoreLogger(channel)->warn(__VA_ARGS__)
#define AX_CORE_ERROR(channel, ...) ::Axle::Log::GetCoreLogger(channel)->error(__VA_ARGS__)
#define AX_CORE_CRITICAL(channel, ...) ::Axle::Log::GetCoreLogger(channel)->critical(__VA_ARGS__)

// Client log macros
#define AX_TRACE(...) ::Axle::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AX_INFO(...) ::Axle::Log::GetClientLogger()->info(__VA_ARGS__)
#define AX_WARN(...) ::Axle::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AX_ERROR(...) ::Axle::Log::GetClientLogger()->error(__VA_ARGS__)
#define AX_CRITICAL(...) ::Axle::Log::GetClientLogger()->critical(__VA_ARGS__)
