#pragma once

#include "axpch.hpp"

#include "Core/Logger/Log.hpp"

#include <format>
#include <source_location>

namespace Axle::PanicDetails {
    struct PanicDynamicStringView {
        template <class T>
            requires std::constructible_from<std::string_view, T>
        PanicDynamicStringView(const T& s, std::source_location loc = std::source_location::current()) noexcept
            : s{s},
              loc{loc} {}

        std::string_view s;
        std::source_location loc;
    };

    template <class... Args>
    struct PanicFormat {
        template <class T>
        consteval PanicFormat(const T& s, std::source_location loc = std::source_location::current()) noexcept
            : fmt{s},
              loc{loc} {}

        std::format_string<Args...> fmt;
        std::source_location loc;
    };

    [[noreturn]] void PanicImpl(const char* s) noexcept;
} // namespace Axle::PanicDetails

namespace Axle {
    [[noreturn]] inline void Panic(PanicDetails::PanicDynamicStringView s) noexcept {
        auto msg = std::format("{}:{} Panic: {}\n", s.loc.file_name(), s.loc.line(), s.s);
        PanicDetails::PanicImpl(msg.c_str());
    }

    template <class... Args>
    [[noreturn]] void Panic(PanicDetails::PanicFormat<std::type_identity_t<Args>...> fmt, Args&&... args) noexcept
        requires(sizeof...(Args) > 0)
    {
        auto msg = std::format("{}:{} Panic: {}\n",
                               fmt.loc.file_name(),
                               fmt.loc.line(),
                               std::format(fmt.fmt, std::forward<Args>(args)...));
        PanicDetails::PanicImpl(msg.c_str());
    }

    [[noreturn]] void TerminateHandler() noexcept;
} // namespace Axle

// TODO: Make a specific AX_CORE_PANIC so that when clients use AX_PANIC they dont have to set the channel as it will be
// LogChannel::Client

// Macros for easier error handling
#define AX_PANIC(channel, ...)               \
    do {                                     \
        AX_CORE_ERROR(channel, __VA_ARGS__); \
        ::Axle::Panic(__VA_ARGS__);          \
    } while (0)

#define AX_ENSURE(condition, ...)  \
    do {                           \
        if (!(condition)) {        \
            AX_PANIC(__VA_ARGS__); \
        }                          \
    } while (0)

// Debug asserts, only active in testing mode
#ifdef AXLE_TESTING
#    define AX_ASSERT(condition, ...)  \
        do {                           \
            if (!(condition)) {        \
                AX_PANIC(__VA_ARGS__); \
            }                          \
        } while (0)
#else
#    define AX_ASSERT(condition, ...) ((void) 0)
#endif // AXLE_TESTING
