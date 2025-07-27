#pragma once

#include "axpch.hpp"
#include "Core/Types.hpp"

#include <cmath>

#include "Core/Core.hpp"

namespace Axle {
	class AXLE_API Mathf {
	public:
		/// The Pi number
		static constexpr f32 PI = 3.14159265358979323846f;
		/// Pi times 2
		static constexpr f32 PI_2 = 2.0f * PI;
		/// Pi times 1/2
		static constexpr f32 PI_1_2 = 0.5f * PI;
		/// Pi times 1/4
		static constexpr f32 PI_1_4 = 0.25f * PI;
		/// One over Pi
		static constexpr f32 PI_1 = 1.0f / PI;
		/// Square root of 2
		static constexpr f32 SQRT_2 = 1.41421356237309504880f;
		/// Square root of 3
		static constexpr f32 SQRT_3 = 1.73205080756887729352f;
		/// Square root of 1/2
		static constexpr f32 SQRT_HALF = 0.70710678118654752440f;
		/// Square root of 1/3
		static constexpr f32 SQRT_THIRD = 0.57735026918962576450f;
		/// Constant to convert from degres to radians
		static constexpr f32 Deg2Rad = PI / 180.0f;
		/// Constant to convert form radians to degress
		static constexpr f32 Rad2Deg = 180.0f / PI;
		/// A number that should be larger than any valid number used
		static constexpr f32 Infinity = 1e30f;
		/// Smallest positive number different form zero
		static constexpr f32 Epsilon = 1.192092896e-07f;

		/// Abs value of a f32
		inline static f32 Abs(f32 f) { return std::abs(f); }
		/// Sine of a f32 (in radians)
		inline static f32 Sin(f32 f) { return std::sin(f); }
		/// Cos of a f32 (in radians)
		inline static f32 Cos(f32 f) { return std::cos(f); }
		/// Tangent of a f32 (in radians)
		inline static f32 Tan(f32 f) { return std::tan(f); }
		/// Arc sinus of a f32 (in radians)
		inline static f32 Asin(f32 f) { return std::asin(f); }
		/// Arc cosinus of a f32 (in radians)
		inline static f32 Acos(f32 f) { return std::acos(f); }
		/// Arc tangent of a f32 (in radians)
		inline static f32 Atan(f32 f) { return std::atan(f); }
		/// Gets the angle (in radians) between the x-axis and the point (x, y)
		inline static f32 Atan2(f32 y, f32 x) { return std::atan2(y, x); }
		/// Square root of a f32
		inline static f32 Sqrt(f32 f) { return std::sqrt(f); }

		/// Returns wheter or not the value is a power of two
		inline static bool IsPowerOfTwo(i32 value) { return (value != 0) && ((value & (value - 1)) == 0); }
		/// Clamps a value between min and max
		inline static f32 Clamp(f32 value, f32 min, f32 max) {
			return value < min ? min : (value > max ? max : value);
		}
		/// Same as Clamp, but clamps between 0 and 1
		inline static f32 Clamp01(f32 value) { return Clamp(value, 0.0f, 1.0f); }

		/// Gets a positive random integer
		static int Random();
		/// Gets a random integer between min and max (inclusive)
		static int Random(i32 min, i32 max);
		/// Gets a random f32 between 0 and 1
		static f32 RandomFloat();
		/// Gets a random f32 between min and max (inclusive)
		static f32 RandomFloat(f32 min, f32 max);

		/// Returns true if the two f32s are approximately equal
		inline static bool Approximately(f32 a, f32 b) { return Abs(a - b) <= Epsilon * 8.0f; }
	};
} // namespace Axle
