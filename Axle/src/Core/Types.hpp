#pragma once

#include <cstdint>

namespace Axle {
	// Unsigned integers
	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;

	// Signed integers
	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	// Floating point
	using f32 = float;
	using f64 = double;

	// Compile-time size checks
	static_assert(sizeof(u8) == 1, "u8 is not 8 bits!");
	static_assert(sizeof(u16) == 2, "u16 is not 16 bits!");
	static_assert(sizeof(u32) == 4, "u32 is not 32 bits!");
	static_assert(sizeof(u64) == 8, "u64 is not 64 bits!");

	static_assert(sizeof(i8) == 1, "i8 is not 8 bits!");
	static_assert(sizeof(i16) == 2, "i16 is not 16 bits!");
	static_assert(sizeof(i32) == 4, "i32 is not 32 bits!");
	static_assert(sizeof(i64) == 8, "i64 is not 64 bits!");

	static_assert(sizeof(f32) == 4, "f32 is not 32 bits!");
	static_assert(sizeof(f64) == 8, "f64 is not 64 bits!");
}
