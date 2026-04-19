#include <doctest.h>
#include <glm/vec2.hpp>

#include "Core/Input/InputManager.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Input/InputState.hpp"

using namespace Axle;

void TestKeyEvents(Event& event) {
    CHECK(event.GetEventCategory() == Axle::EventCategory::Input);
    // CHECK(event->GetContext().u16_values[0] == (unsigned short) Keys::A);
    CHECK(std::get<std::array<u16, 8>>(event.GetContext().raw_data).at(0) == (u16) Keys::A);
}

void TestMouseButtonEvents(Event& event) {
    CHECK(event.GetEventCategory() == Axle::EventCategory::Input);
    // CHECK(event->GetContext().u16_values[0] == (unsigned short) MouseButtons::BUTTON_LEFT);
    CHECK(std::get<std::array<u16, 8>>(event.GetContext().raw_data).at(0) == (u16) MouseButtons::BUTTON_LEFT);
}

void TestMouseWheelEvents(Event& event) {
    CHECK(event.GetEventCategory() == Axle::EventCategory::Input);
    // CHECK(event->GetContext().f32_values[0] == doctest::Approx(1.0));
    CHECK(std::get<std::array<f32, 4>>(event.GetContext().raw_data).at(0) == doctest::Approx(1.0));
}

TEST_CASE("Input system key handling") {
    EventHandler::Init();

    InputManager::Init();
    InputManager::GetInstance().SimulateReset();

    size_t id = EventHandler::GetInstance().Subscribe(TestKeyEvents, EventType::None, EventCategory::Input);

    SUBCASE("No key action") {
        CHECK_FALSE(InputManager::GetInstance().GetKeyDown(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKeyUp(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKey(Keys::A));
    }

    SUBCASE("Key Down") {
        InputManager::GetInstance().SimulateKeyState(Keys::A, true);

        CHECK(InputManager::GetInstance().GetKeyDown(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKey(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKeyUp(Keys::A));

        EventHandler::GetInstance().ProcessEvents();
    }

    SUBCASE("Key Up") {
        InputManager::GetInstance().SimulateKeyState(Keys::A, true);
        EventHandler::GetInstance().ProcessEvents();

        InputManager::GetInstance().SimulateUpdate();
        InputManager::GetInstance().SimulateKeyState(Keys::A, false);
        EventHandler::GetInstance().ProcessEvents();

        CHECK(InputManager::GetInstance().GetKeyUp(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKeyDown(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKey(Keys::A));
    }

    SUBCASE("Key Is Pressed") {
        InputManager::GetInstance().SimulateKeyState(Keys::A, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK_FALSE(InputManager::GetInstance().GetKey(Keys::A));

        InputManager::GetInstance().SimulateUpdate();
        InputManager::GetInstance().SimulateKeyState(Keys::A, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK(InputManager::GetInstance().GetKey(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKeyUp(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKeyDown(Keys::A));
    }

    EventHandler::GetInstance().Unsubscribe(id);
}

TEST_CASE("Input system mouse button handling") {
    EventHandler::Init();

    InputManager::Init();
    InputManager::GetInstance().SimulateReset();

    size_t id = EventHandler::GetInstance().Subscribe(TestMouseButtonEvents, EventType::None, EventCategory::Input);

    SUBCASE("No mouse button action") {
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButton(MouseButtons::BUTTON_LEFT));
    }

    SUBCASE("Mouse button Down") {
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK(InputManager::GetInstance().GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButton(MouseButtons::BUTTON_LEFT));
    }

    SUBCASE("Mouse button Up") {
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
        EventHandler::GetInstance().ProcessEvents();
        InputManager::GetInstance().SimulateUpdate();
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, false);
        EventHandler::GetInstance().ProcessEvents();

        CHECK(InputManager::GetInstance().GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButton(MouseButtons::BUTTON_LEFT));
    }

    SUBCASE("Mouse button Is Pressed") {
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK_FALSE(InputManager::GetInstance().GetMouseButton(MouseButtons::BUTTON_LEFT));

        InputManager::GetInstance().SimulateUpdate();
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
        EventHandler::GetInstance().ProcessEvents();

        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonUp(MouseButtons::BUTTON_LEFT));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
        CHECK(InputManager::GetInstance().GetMouseButton(MouseButtons::BUTTON_LEFT));
    }

    EventHandler::GetInstance().Unsubscribe(id);
}

TEST_CASE("Mouse position handling") {
    EventHandler::Init();

    InputManager::Init();
    InputManager::GetInstance().SimulateReset();

    glm::vec2 testPosition(100.0f, 200.0f);
    InputManager::GetInstance().SimulateMousePosition(testPosition);

    SUBCASE("Get Mouse Position") {
        glm::vec2 mousePos = InputManager::GetInstance().GetMousePosition();

        CHECK(mousePos == testPosition);
    }

    SUBCASE("Set Mouse Position") {
        glm::vec2 newPosition(300.0f, 400.0f);

        InputManager::GetInstance().SimulateMousePosition(newPosition);
        glm::vec2 mousePos = InputManager::GetInstance().GetMousePosition();

        CHECK(mousePos == newPosition);
    }
}

TEST_CASE("Mouse wheel handling") {
    EventHandler::Init();

    InputManager::Init();
    InputManager::GetInstance().SimulateReset();

    size_t id =
        EventHandler::GetInstance().Subscribe(TestMouseWheelEvents, EventType::MouseScrolled, EventCategory::Input);

    InputManager::GetInstance().SimulateMouseWheel(1.0f);
    InputManager::GetInstance().SimulateUpdate();
    EventHandler::GetInstance().ProcessEvents();

    EventHandler::GetInstance().Unsubscribe(id);
}
