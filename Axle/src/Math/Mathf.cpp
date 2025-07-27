#include "axpch.hpp"
#include "Core/Types.hpp"

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

	i32 Mathf::Random() {
		std::uniform_int_distribution<i32> dist(0, RAND_MAX);
		return dist(GetEngine());
	}
	i32 Mathf::Random(i32 min, i32 max) {
		std::uniform_int_distribution<i32> dist(min, max);
		return dist(GetEngine());
	}
	f32 Mathf::RandomFloat() {
		std::uniform_real_distribution<f32> dist(0.0f, 1.0f);
		return dist(GetEngine());
	}
	f32 Mathf::RandomFloat(f32 min, f32 max) {
		std::uniform_real_distribution<f32> dist(min, max);
		return dist(GetEngine());
	}
}
