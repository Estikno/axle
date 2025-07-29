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

		f32* gravity = static_cast<f32*>(res.GetData().at(std::type_index(typeid(f32))).get());

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

	SUBCASE("Add multiple resources") {
		{
			f32* g = new f32;
			std::shared_ptr<i32> iPtr = std::make_shared<i32>(42);

			*g = 9.81f;

			CHECK(res.GetData().empty());

			res.Add<f32>(g);
			res.Add<i32>(iPtr);

			CHECK(res.GetData().size() == 2);
		}

		REQUIRE(res.Contains<f32>());
		REQUIRE(res.Contains<i32>());

		f32* gravity = res.Get<f32>();
		i32* integer = res.Get<i32>();

		REQUIRE_FALSE(gravity == nullptr);
		REQUIRE_FALSE(integer == nullptr);

		CHECK(*gravity == doctest::Approx(9.81f));
		CHECK(*integer == 42);
	}
}