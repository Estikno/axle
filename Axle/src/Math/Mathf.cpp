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
}
