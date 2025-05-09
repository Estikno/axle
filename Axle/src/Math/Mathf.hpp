#pragma once

#include "axpch.hpp"

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

		static float Abs(float f);
		static float Sin(float f);
		static float Cos(float f);
		static float Tan(float f);
		static float Asin(float f);
		static float Acos(float f);
		static float Atan(float f);
		static float Atan2(float y, float x);
		static float Sqrt(float f);

		static inline bool IsPowerOfTwo(int value) {
			return (value != 0) && ((value & (value - 1)) == 0);
		}
		static inline float Clamp(float value, float min, float max);
		static inline float Clamp01(float value) {
			return Clamp(value, 0.0f, 1.0f);
		}

		static int Random();
		static int Random(int min, int max);
		static float RandomFloat();
		static float RandomFloat(float min, float max);

		static bool Approximately(float a, float b) {
			return Abs(a - b) <= Epsilon;
		}
	};
}
