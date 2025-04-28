#include "Log.hpp"
#include "../Core.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Axle {
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init() {
        if (s_CoreLogger != nullptr) {
            AX_CORE_WARN("Init method of the logger has been called a second time. IGNORING");
            return;
        }

        // Custom pattern
        // Se for more information: https://github.com/gabime/spdlog/wiki/Custom-formatting#customizing-format-using-set_pattern
        spdlog::set_pattern("%^[%T] %n: %v%$");

        // Creates two multithread safe loggers
        s_CoreLogger = spdlog::stdout_color_mt("AXLE");
        s_CoreLogger->set_level(spdlog::level::trace);

        s_ClientLogger = spdlog::stdout_color_mt("APP");
        s_ClientLogger->set_level(spdlog::level::trace);

        AX_CORE_TRACE("Logger initialized...");
    }
} 
