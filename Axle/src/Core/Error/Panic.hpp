#pragma once

#include "axpch.hpp"

#include <concepts>
#include <format>
#include <source_location>
#include <string_view>
#include <type_traits>

namespace Axle::Error {
	struct PanicDynamicStringView {
		template <class T>
			requires std::constructible_from<std::string_view, T>
		PanicDynamicStringView(
			const T& s,
			std::source_location loc = std::source_location::current()) noexcept
			: s{ s }, loc{ loc } {
		}

		std::string_view s;
		std::source_location loc;
	};

	template <class... Args>
	struct PanicFormat {
		template <class T>
		consteval PanicFormat(
			const T& s,
			std::source_location loc = std::source_location::current()) noexcept
			: fmt{ s }, loc{ loc } {
		}

		std::format_string<Args...> fmt;
		std::source_location loc;
	};

	[[noreturn]] void PanicImpl(const char* s) noexcept;
}

namespace Axle {
	[[noreturn]] inline void Panic(Error::PanicDynamicStringView s) noexcept {
		auto msg =
			std::format("{}:{} Panic: {}\n", s.loc.file_name(), s.loc.line(), s.s);
		Error::PanicImpl(msg.c_str());
	}

	template <class... Args>
	[[noreturn]] void Panic(Error::PanicFormat<std::type_identity_t<Args>...> fmt, Args &&...args) noexcept
		requires (sizeof ...(Args) > 0)
	{
		auto msg = std::format("{}:{} Panic: {}\n", fmt.loc.file_name(), fmt.loc.line(),
			std::format(fmt.fmt, std::forward<Args>(args)...));
		Error::PanicImpl(msg.c_str());
	}
}
