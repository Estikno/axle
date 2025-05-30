#pragma once

#include "axpch.hpp"

#include <cmath>

#include "Core/Core.hpp"

namespace Axle {
	class AXLE_API Mathf {
	public:
		/// The Pi number
		static constexpr float PI = 3.14159265358979323846f;
		/// Pi times 2
		static constexpr float PI_2 = 2.0f * PI;
		/// Pi times 1/2
		static constexpr float PI_1_2 = 0.5f * PI;
		/// Pi times 1/4
		static constexpr float PI_1_4 = 0.25f * PI;
		/// One over Pi
		static constexpr float PI_1 = 1.0f / PI;
		/// Square root of 2
		static constexpr float SQRT_2 = 1.41421356237309504880f;
		/// Square root of 3
		static constexpr float SQRT_3 = 1.73205080756887729352f;
		/// Square root of 1/2
		static constexpr float SQRT_HALF = 0.70710678118654752440f;
		/// Square root of 1/3
		static constexpr float SQRT_THIRD = 0.57735026918962576450f;
		/// Constant to convert from degres to radians
		static constexpr float Deg2Rad = PI / 180.0f;
		/// Constant to convert form radians to degress
		static constexpr float Rad2Deg = 180.0f / PI;
		/// A number that should be larger than any valid number used
		static constexpr float Infinity = 1e30f;
		/// Smallest positive number different form zero
		static constexpr float Epsilon = 1.192092896e-07f;

		/// Abs value of a float
		inline static float Abs(float f) { return std::abs(f); }
		/// Sine of a float (in radians)
		inline static float Sin(float f) { return std::sin(f); }
		/// Cos of a float (in radians)
		inline static float Cos(float f) { return std::cos(f); }
		/// Tangent of a float (in radians)
		inline static float Tan(float f) { return std::tan(f); }
		/// Arc sinus of a float (in radians)
		inline static float Asin(float f) { return std::asin(f); }
		/// Arc cosinus of a float (in radians)
		inline static float Acos(float f) { return std::acos(f); }
		/// Arc tangent of a float (in radians)
		inline static float Atan(float f) { return std::atan(f); }
		/// Gets the angle (in radians) between the x-axis and the point (x, y)
		inline static float Atan2(float y, float x) { return std::atan2(y, x); }
		/// Square root of a float
		inline static float Sqrt(float f) { return std::sqrt(f); }

		/// Returns wheter or not the value is a power of two
		inline static bool IsPowerOfTwo(int value) { return (value != 0) && ((value & (value - 1)) == 0); }
		/// Clamps a value between min and max
		inline static float Clamp(float value, float min, float max) {
			return value < min ? min : (value > max ? max : value);
		}
		/// Same as Clamp, but clamps between 0 and 1
		inline static float Clamp01(float value) { return Clamp(value, 0.0f, 1.0f); }

		/// Gets a positive random integer
		static int Random();
		/// Gets a random integer between min and max (inclusive)
		static int Random(int min, int max);
		/// Gets a random float between 0 and 1
		static float RandomFloat();
		/// Gets a random float between min and max (inclusive)
		static float RandomFloat(float min, float max);

		/// Returns true if the two floats are approximately equal
		inline static bool Approximately(float a, float b) { return Abs(a - b) <= Epsilon * 8.0f; }
	};
} // namespace Axle
