#include "doctest.h"

#include "Math/Mathf.hpp"
#include "Math/Math_Types.hpp"

using namespace Axle;

TEST_CASE("Mathf") {
	SUBCASE("Mathf::Abs") {
		CHECK(Mathf::Abs(0.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Abs(1.0f) == doctest::Approx(1.0f));
		CHECK(Mathf::Abs(-1.0f) == doctest::Approx(1.0f));
		CHECK(Mathf::Abs(-10.5f) == doctest::Approx(10.5f));
		CHECK(Mathf::Abs(-103.85f) == doctest::Approx(103.85f));
	}

	SUBCASE("Mathf::Sin") {
		CHECK(Mathf::Sin(0.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Sin(Mathf::PI_1_2) == doctest::Approx(1.0f));
		CHECK(Mathf::Sin(Mathf::PI) == doctest::Approx(0.0f));
		CHECK(Mathf::Sin(Mathf::PI_1_4) == doctest::Approx(0.70710678118f));
		CHECK(Mathf::Sin(-Mathf::PI_1_4) == doctest::Approx(-0.70710678118f));
	}

	SUBCASE("Mathf::Cos") {
		CHECK(Mathf::Cos(0.0f) == doctest::Approx(1.0f));
		CHECK(Mathf::Cos(Mathf::PI_1_2) == doctest::Approx(0.0f));
		CHECK(Mathf::Cos(Mathf::PI) == doctest::Approx(-1.0f));
		CHECK(Mathf::Cos(Mathf::PI_1_4) == doctest::Approx(0.70710678118f));
		CHECK(Mathf::Cos(-Mathf::PI_1_4) == doctest::Approx(0.70710678118f));
	}

	SUBCASE("Mathf::Tan") {
		CHECK(Mathf::Tan(0.0f) == doctest::Approx(0.0f));
		//CHECK(Mathf::Tan(Mathf::PI_1_2) == doctest::Approx(INFINITY));
		CHECK(Mathf::Tan(Mathf::PI) == doctest::Approx(0.0f));
		CHECK(Mathf::Tan(Mathf::PI_1_4) == doctest::Approx(1.0f));
		CHECK(Mathf::Tan(-Mathf::PI_1_4) == doctest::Approx(-1.0f));
	}

	SUBCASE("Mathf::Asin") {
		CHECK(Mathf::Asin(0.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Asin(1.0f) == doctest::Approx(Mathf::PI_1_2));
		CHECK(Mathf::Asin(-1.0f) == doctest::Approx(-Mathf::PI_1_2));
		CHECK(Mathf::Asin(0.5f) == doctest::Approx(0.5235987756f));
		CHECK(Mathf::Asin(-0.5f) == doctest::Approx(-0.5235987756f));
	}

	SUBCASE("Mathf::Acos") {
		CHECK(Mathf::Acos(1.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Acos(-1.0f) == doctest::Approx(Mathf::PI));
		CHECK(Mathf::Acos(0.5f) == doctest::Approx(1.0471975512f));
		CHECK(Mathf::Acos(-0.5f) == doctest::Approx(2.0943951024f));
		CHECK(Mathf::Acos(0.0f) == doctest::Approx(Mathf::PI_1_2));
	}

	SUBCASE("Mathf::Atan") {
		CHECK(Mathf::Atan(0.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Atan(1.0f) == doctest::Approx(Mathf::PI_1_4));
		CHECK(Mathf::Atan(-1.0f) == doctest::Approx(-Mathf::PI_1_4));
		CHECK(Mathf::Atan(2.0f) == doctest::Approx(1.1071487178f));
		CHECK(Mathf::Atan(-2.0f) == doctest::Approx(-1.1071487178f));
	}

	SUBCASE("Mathf::Atan2") {
		CHECK(Mathf::Atan2(0.0f, 1.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Atan2(1.0f, 1.0f) == doctest::Approx(Mathf::PI_1_4));
		CHECK(Mathf::Atan2(-1.0f, 1.0f) == doctest::Approx(-Mathf::PI_1_4));
		CHECK(Mathf::Atan2(2.0f, 1.0f) == doctest::Approx(1.1071487178f));
		CHECK(Mathf::Atan2(-2.0f, 1.0f) == doctest::Approx(-1.1071487178f));
	}

	SUBCASE("Mathf::Sqrt") {
		CHECK(Mathf::Sqrt(0.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Sqrt(1.0f) == doctest::Approx(1.0f));
		CHECK(Mathf::Sqrt(4.0f) == doctest::Approx(2.0f));
		CHECK(Mathf::Sqrt(9.0f) == doctest::Approx(3.0f));
		CHECK(Mathf::Sqrt(16.0f) == doctest::Approx(4.0f));
	}

	SUBCASE("Mathf::IsPowerOfTwo") {
		CHECK(Mathf::IsPowerOfTwo(1) == true);
		CHECK(Mathf::IsPowerOfTwo(2) == true);
		CHECK(Mathf::IsPowerOfTwo(3) == false);
		CHECK(Mathf::IsPowerOfTwo(4) == true);
		CHECK(Mathf::IsPowerOfTwo(5) == false);
		CHECK(Mathf::IsPowerOfTwo(6) == false);
		CHECK(Mathf::IsPowerOfTwo(7) == false);
		CHECK(Mathf::IsPowerOfTwo(8) == true);
	}

	SUBCASE("Mathf::Clamp") {
		CHECK(Mathf::Clamp(0.0f, 0.0f, 1.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Clamp(1.0f, 0.0f, 1.0f) == doctest::Approx(1.0f));
		CHECK(Mathf::Clamp(2.0f, 0.0f, 1.0f) == doctest::Approx(1.0f));
		CHECK(Mathf::Clamp(-1.0f, 0.0f, 1.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Clamp(5.5f, 2.5f, 4.5f) == doctest::Approx(4.5f));
	}

	SUBCASE("Mathf::Clamp01") {
		CHECK(Mathf::Clamp01(0.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Clamp01(1.0f) == doctest::Approx(1.0f));
		CHECK(Mathf::Clamp01(2.0f) == doctest::Approx(1.0f));
		CHECK(Mathf::Clamp01(-1.0f) == doctest::Approx(0.0f));
		CHECK(Mathf::Clamp01(5.5f) == doctest::Approx(1.0f));
	}

	SUBCASE("Mathf::Random") {
		CHECK(Mathf::Random() >= 0);
		CHECK(Mathf::Random() <= 32767);
		CHECK(Mathf::Random(1, 10) >= 1);
		CHECK(Mathf::Random(1, 10) <= 10);
		CHECK(Mathf::Random(5, 15) >= 5);
		CHECK(Mathf::Random(5, 15) <= 15);
	}

	SUBCASE("Mathf::RandomFloat") {
		CHECK(Mathf::RandomFloat() >= 0.0f);
		CHECK(Mathf::RandomFloat() <= 1.0f);
		CHECK(Mathf::RandomFloat(1.0f, 10.0f) >= 1.0f);
		CHECK(Mathf::RandomFloat(1.0f, 10.0f) <= 10.0f);
		CHECK(Mathf::RandomFloat(5.0f, 15.0f) >= 5.0f);
		CHECK(Mathf::RandomFloat(5.0f, 15.0f) <= 15.0f);
	}

	SUBCASE("Mathf::Approximately") {
		CHECK(Mathf::Approximately(0.0f, 0.0f) == true);
		CHECK(Mathf::Approximately(1.0f, 1.0f) == true);
		CHECK(Mathf::Approximately(-1.0f, -1.0f) == true);
		CHECK(Mathf::Approximately(1.0f, 1.00001f) == false);
		CHECK(Mathf::Approximately(1.00001f, 1.0f) == false);
	}
}

TEST_CASE("Vector2") {
	SUBCASE("Constructors and Constants") {
		CHECK(Vector2::Zero() == Vector2(0.0f, 0.0f));
		CHECK(Vector2::One() == Vector2(1.0f, 1.0f));
		CHECK(Vector2::Right() == Vector2(1.0f, 0.0f));
		CHECK(Vector2::Left() == Vector2(-1.0f, 0.0f));
		CHECK(Vector2::Up() == Vector2(0.0f, 1.0f));
		CHECK(Vector2::Down() == Vector2(0.0f, -1.0f));
	}

	SUBCASE("Magnitude and SqrMagnitude") {
		Vector2 v(3.0f, 4.0f);
		CHECK(v.SqrMagnitude() == doctest::Approx(25.0f));
		CHECK(v.Magnitude() == doctest::Approx(5.0f));
	}

	SUBCASE("Normalize and Normalized") {
		Vector2 v(3.0f, 4.0f);
		Vector2 n = v.Normalized();
		CHECK(n.Magnitude() == doctest::Approx(1.0f));
		CHECK(n == Vector2(0.6f, 0.8f));

		v.Normalize();
		CHECK(v == Vector2(0.6f, 0.8f));
	}

	SUBCASE("Perpendicular") {
		Vector2 v(1.0f, 0.0f);
		CHECK(v.Perpendicular() == Vector2(0.0f, 1.0f));
	}

	SUBCASE("Operators") {
		Vector2 a(1.0f, 2.0f);
		Vector2 b(3.0f, 4.0f);

		CHECK((a + b) == Vector2(4.0f, 6.0f));
		CHECK((b - a) == Vector2(2.0f, 2.0f));
		CHECK((a * 2.0f) == Vector2(2.0f, 4.0f));
		CHECK((b / 2.0f) == Vector2(1.5f, 2.0f));
	}

	SUBCASE("Dot and Distance") {
		Vector2 a(1.0f, 0.0f);
		Vector2 b(0.0f, 1.0f);

		CHECK(Vector2::Dot(a, b) == doctest::Approx(0.0f));
		CHECK(Vector2::Dot(a, a) == doctest::Approx(1.0f));
		CHECK(Vector2::Distance(a, b) == doctest::Approx(std::sqrt(2.0f)));
	}

	SUBCASE("Angle between vectors") {
		Vector2 a(1.0f, 0.0f);
		Vector2 b(0.0f, 1.0f);
		CHECK(Vector2::Angle(a, b) == doctest::Approx(Mathf::PI / 2.0f)); // assumes PI is defined
	}

	SUBCASE("Lerp and LerpUnclamped") {
		Vector2 a(0.0f, 0.0f);
		Vector2 b(2.0f, 2.0f);

		CHECK(Vector2::Lerp(a, b, 0.5f) == Vector2(1.0f, 1.0f));
		CHECK(Vector2::LerpUnclamped(a, b, 1.5f) == Vector2(3.0f, 3.0f));
	}

	SUBCASE("Reflect") {
		Vector2 dir(1.0f, -1.0f);
		Vector2 normal(0.0f, 1.0f);
		CHECK(Vector2::Reflect(dir, normal) == Vector2(1.0f, 1.0f));
	}

	SUBCASE("ScalarProjection and Project") {
		Vector2 a(2.0f, 3.0f);
		Vector2 b(4.0f, 0.0f);

		CHECK(Vector2::ScalarProjection(a, b) == doctest::Approx(2.0f));
		CHECK(Vector2::Project(a, b) == Vector2(2.0f, 0.0f));
	}
}