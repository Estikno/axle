#include "axpch.hpp"

#include <cmath>
#include <random>

#include "Mathf.hpp"

namespace Axle {
	namespace {
		std::mt19937& GetEngine() {
			static std::random_device rd;
			static std::mt19937 engine(rd());
			return engine;
		}
	}

	float Mathf::Abs(float f) { return std::abs(f); }
	float Mathf::Sin(float f) { return std::sin(f); }
	float Mathf::Cos(float f) { return std::cos(f); }
	float Mathf::Tan(float f) { return std::tan(f); }
	float Mathf::Asin(float f) { return std::asin(f); }
	float Mathf::Acos(float f) { return std::acos(f); }
	float Mathf::Atan(float f) { return std::atan(f); }
	float Mathf::Atan2(float y, float x) { return std::atan2(y, x); }
	float Mathf::Sqrt(float f) { return std::sqrt(f); }

	int Mathf::Random() {
		std::uniform_int_distribution<int> dist(0, RAND_MAX);
		return dist(GetEngine());
	}
	int Mathf::Random(int min, int max) {
		std::uniform_int_distribution<int> dist(min, max);
		return dist(GetEngine());
	}
	float Mathf::RandomFloat() {
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		return dist(GetEngine());
	}
	float Mathf::RandomFloat(float min, float max) {
		std::uniform_real_distribution<float> dist(min, max);
		return dist(GetEngine());
	}

	float Mathf::Clamp(float value, float min, float max) {
		return value < min ? min : (value > max ? max : value);
	}
}
