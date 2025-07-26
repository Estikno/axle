#include <doctest.h>

#include "Core/Input/Input.hpp"

using namespace Axle;

TEST_CASE("Input system key handling") {
	Input::SimulateReset();

	SUBCASE("No key action") {
		CHECK_FALSE(Input::GetKeyDown(Keys::A));
		CHECK_FALSE(Input::GetKeyUp(Keys::A));
		CHECK_FALSE(Input::GetKey(Keys::A));
	}

	SUBCASE("Key Down") {
		Input::SimulateKeyState(Keys::A, true);

		CHECK(Input::GetKeyDown(Keys::A));
		CHECK_FALSE(Input::GetKey(Keys::A));
		CHECK_FALSE(Input::GetKeyUp(Keys::A));
	}

	SUBCASE("Key Up") {
		Input::SimulateKeyState(Keys::A, true);
		Input::SimulateUpdate();
		Input::SimulateKeyState(Keys::A, false);

		CHECK(Input::GetKeyUp(Keys::A));
		CHECK_FALSE(Input::GetKeyDown(Keys::A));
		CHECK_FALSE(Input::GetKey(Keys::A));
	}

	SUBCASE("Key Is Pressed") {
		Input::SimulateKeyState(Keys::A, true);

		CHECK_FALSE(Input::GetKey(Keys::A));

		Input::SimulateUpdate();
		Input::SimulateKeyState(Keys::A, true);

		CHECK(Input::GetKey(Keys::A));
		CHECK_FALSE(Input::GetKeyUp(Keys::A));
		CHECK_FALSE(Input::GetKeyDown(Keys::A));
	}
}

TEST_CASE("Input system mouse button handling") {
	Input::SimulateReset();

	SUBCASE("No mouse button action") {
		CHECK_FALSE(Input::GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
		CHECK_FALSE(Input::GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
		CHECK_FALSE(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));
	}

	SUBCASE("Mouse button Down") {
		Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);

		CHECK(Input::GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
		CHECK_FALSE(Input::GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
		CHECK_FALSE(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));
	}

	SUBCASE("Mouse button Up") {
		Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
		Input::SimulateUpdate();
		Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, false);

		CHECK(Input::GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
		CHECK_FALSE(Input::GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
		CHECK_FALSE(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));
	}

	SUBCASE("Mouse button Is Pressed") {
		Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);

		CHECK_FALSE(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));

		Input::SimulateUpdate();
		Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);

		CHECK_FALSE(Input::GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
		CHECK_FALSE(Input::GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
		CHECK(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));
	}
}
