#include <doctest.h>
#include <glm/vec2.hpp>

#include "Core/Input/Input.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Input/InputState.hpp"

using namespace Axle;

void TestKeyEvents(Event* event) {
    CHECK_FALSE(event == nullptr);
    CHECK(event->GetEventCategory() == Axle::EventCategory::Input);
    // CHECK(event->GetContext().u16_values[0] == (unsigned short) Keys::A);
    CHECK(std::get<std::array<u16, 8>>(event->GetContext().raw_data).at(0) == (u16) Keys::A);
}

void TestMouseButtonEvents(Event* event) {
    CHECK_FALSE(event == nullptr);
    CHECK(event->GetEventCategory() == Axle::EventCategory::Input);
    // CHECK(event->GetContext().u16_values[0] == (unsigned short) MouseButtons::BUTTON_LEFT);
    CHECK(std::get<std::array<u16, 8>>(event->GetContext().raw_data).at(0) == (u16) MouseButtons::BUTTON_LEFT);
}

void TestMouseWheelEvents(Event* event) {
    CHECK_FALSE(event == nullptr);
    CHECK(event->GetEventCategory() == Axle::EventCategory::Input);
    // CHECK(event->GetContext().f32_values[0] == doctest::Approx(1.0));
    CHECK(std::get<std::array<f32, 4>>(event->GetContext().raw_data).at(0) == doctest::Approx(1.0));
}

TEST_CASE("Input system key handling") {
    Input::SimulateReset();
    EventHandler::Init();

    size_t id = EventHandler::GetInstance().Subscribe(TestKeyEvents, EventType::None, EventCategory::Input);

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

        EventHandler::GetInstance().ProcessEvents();
    }

    SUBCASE("Key Up") {
        Input::SimulateKeyState(Keys::A, true);
        EventHandler::GetInstance().ProcessEvents();
        Input::SimulateUpdate();
        Input::SimulateKeyState(Keys::A, false);
        EventHandler::GetInstance().ProcessEvents();

        CHECK(Input::GetKeyUp(Keys::A));
        CHECK_FALSE(Input::GetKeyDown(Keys::A));
        CHECK_FALSE(Input::GetKey(Keys::A));
    }

    SUBCASE("Key Is Pressed") {
        Input::SimulateKeyState(Keys::A, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK_FALSE(Input::GetKey(Keys::A));

        Input::SimulateUpdate();
        Input::SimulateKeyState(Keys::A, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK(Input::GetKey(Keys::A));
        CHECK_FALSE(Input::GetKeyUp(Keys::A));
        CHECK_FALSE(Input::GetKeyDown(Keys::A));
    }

    EventHandler::GetInstance().Unsubscribe(id);
}

TEST_CASE("Input system mouse button handling") {
    Input::SimulateReset();
    EventHandler::Init();

    size_t id = EventHandler::GetInstance().Subscribe(TestMouseButtonEvents, EventType::None, EventCategory::Input);

    SUBCASE("No mouse button action") {
        CHECK_FALSE(Input::GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(Input::GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));
    }

    SUBCASE("Mouse button Down") {
        Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK(Input::GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(Input::GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));
    }

    SUBCASE("Mouse button Up") {
        Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
        EventHandler::GetInstance().ProcessEvents();
        Input::SimulateUpdate();
        Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, false);
        EventHandler::GetInstance().ProcessEvents();

        CHECK(Input::GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(Input::GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));
    }

    SUBCASE("Mouse button Is Pressed") {
        Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK_FALSE(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));

        Input::SimulateUpdate();
        Input::SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK_FALSE(Input::GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(Input::GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
        CHECK(Input::GetMouseButton(MouseButtons::BUTTON_LEFT));
    }

    EventHandler::GetInstance().Unsubscribe(id);
}

TEST_CASE("Mouse position handling") {
    Input::SimulateReset();
    EventHandler::Init();

    glm::vec2 testPosition(100.0f, 200.0f);
    Input::SimulateMousePosition(testPosition);

    SUBCASE("Get Mouse Position") {
        glm::vec2 mousePos = Input::GetMousePosition();

        CHECK(mousePos == testPosition);
    }

    SUBCASE("Set Mouse Position") {
        glm::vec2 newPosition(300.0f, 400.0f);

        Input::SimulateMousePosition(newPosition);
        glm::vec2 mousePos = Input::GetMousePosition();

        CHECK(mousePos == newPosition);
    }
}

TEST_CASE("Mouse wheel handling") {
    Input::SimulateReset();
    EventHandler::Init();

    size_t id =
        EventHandler::GetInstance().Subscribe(TestMouseWheelEvents, EventType::MouseScrolled, EventCategory::Input);

    Input::SimulateMouseWheel(1.0f);
    Input::SimulateUpdate();
    EventHandler::GetInstance().ProcessEvents();

    EventHandler::GetInstance().Unsubscribe(id);
}
