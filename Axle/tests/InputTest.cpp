#include <doctest.h>

#include "Math/Mathf.hpp"

#include "Core/Input/Input.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"

using namespace Axle;

void TestKeyEvents(Event* event) {
	CHECK_FALSE(event == nullptr);
	CHECK(event->GetEventCategory() == Axle::EventCategory::Input);
	CHECK(event->GetContext().u16_values[0] == (unsigned short)Keys::A);
}

void TestMouseButtonEvents(Event* event) {
	CHECK_FALSE(event == nullptr);
	CHECK(event->GetEventCategory() == Axle::EventCategory::Input);
	CHECK(event->GetContext().u16_values[0] == (unsigned short)MouseButtons::BUTTON_LEFT);
}

void TestMouseWheelEvents(Event* event) {
	CHECK_FALSE(event == nullptr);
	CHECK(event->GetEventCategory() == Axle::EventCategory::Input);
	CHECK(Mathf::Approximately(event->GetContext().f32_values[0], 1.0f));
}

TEST_CASE("Input system key handling") {
	Input::SimulateReset();
	EventHandler::Init();

	Subscription sub_1 = EventHandler::GetInstance().Subscribe(TestKeyEvents, EventType::None, EventCategory::Input);

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
	EventHandler::Init();

	Subscription sub_1 = EventHandler::GetInstance().Subscribe(TestMouseButtonEvents, EventType::None, EventCategory::Input);

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

TEST_CASE("Mouse position handling") {
	Input::SimulateReset();
	EventHandler::Init();

	Vector2 testPosition(100.0f, 200.0f);
	Input::SimulateMousePosition(testPosition);

	SUBCASE("Get Mouse Position") {
		Vector2 mousePos = Input::GetMousePosition();

		CHECK(mousePos == testPosition);
	}

	SUBCASE("Set Mouse Position") {
		Vector2 newPosition(300.0f, 400.0f);

		Input::SimulateMousePosition(newPosition);
		Vector2 mousePos = Input::GetMousePosition();

		CHECK(mousePos == newPosition);
	}
}

TEST_CASE("Mouse wheel handling") {
	Input::SimulateReset();
	EventHandler::Init();

	Subscription sub_1 = EventHandler::GetInstance().Subscribe(TestMouseWheelEvents, EventType::None, EventCategory::Input);

	Input::SimulateMouseWheel(1.0f);
}
