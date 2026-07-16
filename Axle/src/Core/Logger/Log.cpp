#include "axpch.hpp"

#include "Log.hpp"
#include "../Core.hpp"

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Axle {
    std::unique_ptr<Log> Log::s_Instance = nullptr;

    void Log::Init() {
        if (s_Instance != nullptr) {
            AX_CORE_WARN(LogChannel::Core, "Init method of the logger has been called a second time. IGNORING");
            return;
        }

        // Custom pattern
        // Se for more information:
        // https://github.com/gabime/spdlog/wiki/Custom-formatting#customizing-format-using-set_pattern
        // spdlog::set_pattern("%^[%T] %n: %v%$");

        s_Instance = std::make_unique<Log>();

        // Assign sinks
        s_Instance->m_ConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        s_Instance->m_ConsoleSink->set_pattern("%^[%T] %n: %v%$");

        s_Instance->m_FileSink =
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>("assets/tests/logs.log", 1024 * 1024, 5);
        s_Instance->m_FileSink->set_pattern("%^[%T] %n: %v%$");

        // Assign loggers
        for (u8 i = 0; i < static_cast<u8>(LogChannel::MaxChannels); ++i) {
            s_Instance->m_ChannelLoggers[i] = std::make_shared<spdlog::logger>(
                std::string(CHANNEL_NAMES[i]),
                spdlog::sinks_init_list{s_Instance->m_ConsoleSink, s_Instance->m_FileSink});
            s_Instance->m_ChannelLoggers[i]->set_level(spdlog::level::trace);
        }


        AX_CORE_INFO(LogChannel::Core, "Logger initialized...");
    }

    void Log::ShutDown() {
        AX_CORE_INFO(LogChannel::Core, "Logger deleted...");

        // s_Instance->m_CoreLogger.reset();
        // s_Instance->m_ClientLogger.reset();
        s_Instance.reset();
    }

    void Log::SetVerbosityImpl(LogVerbosity verbosity) {
        m_Verbosity.store(verbosity, std::memory_order_release);
        for (u8 i = 0; i < static_cast<u8>(LogChannel::MaxChannels); ++i) {
            m_ChannelLoggers[i]->set_level(ToSpdlogLevel(verbosity));
        }
    }
} // namespace Axle
