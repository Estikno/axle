#include <doctest.h>

#include <any>
#include <typeindex>
#include <typeinfo>
#include <memory>

#include "Core/Types.hpp"
#include "ECS/Resources.hpp"
#include "Math/Mathf.hpp"
#include "Core/Logger/Log.hpp"

using namespace Axle;

TEST_CASE("Resources") {
	Resources res;
	Log::Init();

	SUBCASE("Add resource") {
		f32* g = new f32;
		*g = 9.81f;
		res.Add<f32>(g);

		REQUIRE(res.GetData().contains(std::type_index(typeid(f32))));
		CHECK(res.Contains<f32>());

		ResourceEntry& data = res.GetData().at(std::type_index(typeid(f32)));
		f32* gravity = std::any_cast<f32*>(data.ptr);

		REQUIRE_FALSE(gravity == nullptr);
		CHECK(*gravity == doctest::Approx(9.81f));
	}

	SUBCASE("Get resource") {
		f32* g = new f32;
		*g = 9.81f;
		res.Add<f32>(g);

		f32* gravity = res.Get<f32>();

		REQUIRE_FALSE(gravity == nullptr);
		CHECK(*gravity == doctest::Approx(9.81f));
	}

	SUBCASE("Get resource on another context") {
		{
			f32* g = new f32;
			*g = 9.81f;
			res.Add<f32>(g);
		}

		f32* gravity = res.Get<f32>();

		REQUIRE_FALSE(gravity == nullptr);
		CHECK(*gravity == doctest::Approx(9.81f));
	}

	SUBCASE("Modify resource") {
		{
			f32* g = new f32;
			*g = 9.81f;
			res.Add<f32>(g);
		}

		{
			f32* gravity = res.Get<f32>();

			*gravity = 10.0f;
		}


		f32* gravity = res.Get<f32>();

		REQUIRE_FALSE(gravity == nullptr);
		CHECK(*gravity == doctest::Approx(10.0f));
	}

	SUBCASE("Remove resource") {
		{
			f32* g = new f32;
			*g = 9.81f;
			res.Add<f32>(g);
		}

		{
			res.Remove<f32>();
		}

		f32* gravity = res.Get<f32>();
		size_t size = res.GetData().size();

		CHECK(gravity == nullptr);
		CHECK(size == 0);
	}
}