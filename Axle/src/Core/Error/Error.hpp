#pragma once

#include "axpch.hpp"

#include <source_location>
#include <spdlog/fmt/fmt.h>

#include "Core/Types.hpp"

namespace Axle {
    enum class ErrorCode : u32 {
        Unknown = 0,
        InvalidArgument,
        OutOfRange,
        NotFound,
        IOError,
        ParseError,
        OutOfMemory,
        ShaderCompileFailed,
        AssetLoadFailed,
        GLError,
    };

    inline constexpr std::string_view ToString(ErrorCode code) noexcept {
        switch (code) {
            case ErrorCode::Unknown:
                return "Unknown";
            case ErrorCode::InvalidArgument:
                return "InvalidArgument";
            case ErrorCode::OutOfRange:
                return "OutOfRange";
            case ErrorCode::NotFound:
                return "NotFound";
            case ErrorCode::IOError:
                return "IOError";
            case ErrorCode::ParseError:
                return "ParseError";
            case ErrorCode::OutOfMemory:
                return "OutOfMemory";
            case ErrorCode::ShaderCompileFailed:
                return "ShaderCompileFailed";
            case ErrorCode::AssetLoadFailed:
                return "AssetLoadFailed";
            case ErrorCode::GLError:
                return "GLError";
        }
        return "???";
    }

    struct Error {
        ErrorCode code;
        std::string message;
        std::source_location location;

        Error(ErrorCode code,
              const std::string& message,
              std::source_location location = std::source_location::current()) noexcept
            : code(code),
              message(std::move(message)),
              location(location) {}
    };
} // namespace Axle

// Lets AX_PANIC / std::format treat an Axle::Error as a formattable argument directly,
// e.g. AX_PANIC(LogChannel::Core, "unwrap failed: {}", err);
template <>
struct fmt::formatter<Axle::Error> : fmt::formatter<std::string_view> {
    auto format(const Axle::Error& e, format_context& ctx) const {
        return fmt::formatter<std::string_view>::format(
            fmt::format(
                "[{}] {} ({}:{})", Axle::ToString(e.code), e.message, e.location.file_name(), e.location.line()),
            ctx);
    }
};
template <>
struct std::formatter<Axle::Error> : std::formatter<std::string_view> {
    auto format(const Axle::Error& e, std::format_context& ctx) const {
        auto out = std::format(
            "[{}] {} ({}:{})", Axle::ToString(e.code), e.message, e.location.file_name(), e.location.line());
        return std::formatter<std::string_view>::format(out, ctx);
    }
};
