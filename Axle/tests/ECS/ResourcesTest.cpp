#include <doctest.h>

#include <any>
#include <typeindex>
#include <typeinfo>
#include <memory>

#include "Core/Types.hpp"
#include "ECS/Resources.hpp"
#include "Math/Mathf.hpp"

using namespace Axle;

TEST_CASE("Resources") {
	Resources res;

	SUBCASE("Add resource") {
		f32* g = new f32;
		*g = 9.81f;
		res.Add<f32>(g);

		f32* gravity = res.Get<f32>();

		REQUIRE_FALSE(gravity == nullptr);
		CHECK(*gravity == doctest::Approx(9.81f));
	}
}