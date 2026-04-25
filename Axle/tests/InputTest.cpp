#include <doctest.h>
#include <glm/vec2.hpp>

#include "Core/Input/InputManager.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Logger/Log.hpp"

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
// ─── Additional edge cases for InputManager ───────────────────────────────

// ---------------------------------------------------------------------------
// Helpers / fixtures
// ---------------------------------------------------------------------------

// Sets up both managers and resets input state. Call at the top of each TEST_CASE.
struct InputFixture {
    InputFixture() {
        Log::Init();
        EventHandler::Init();
        InputManager::Init();
        InputManager::GetInstance().SimulateReset();
    }
    ~InputFixture() {
        InputManager::ShutDown();
        EventHandler::ShutDown();
    }
};

// ---------------------------------------------------------------------------
// 1. Singleton lifecycle
// ---------------------------------------------------------------------------

TEST_CASE("InputManager singleton lifecycle") {
    Log::Init();
    EventHandler::Init();

    InputManager::Init();
    InputManager& first = InputManager::GetInstance();
    InputManager::Init(); // second call should warn and be a no-op
    InputManager& second = InputManager::GetInstance();

    CHECK(&first == &second);

    InputManager::ShutDown();

    // Re-init after shutdown must work cleanly
    InputManager::Init();
    CHECK_NOTHROW(InputManager::GetInstance());
    InputManager::ShutDown();

    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 2. SetKey deduplication — no event fired on redundant press
// ---------------------------------------------------------------------------

TEST_CASE("InputManager SetKey deduplication") {
    InputFixture fix;

    int eventCount = 0;
    size_t id = EventHandler::GetInstance().Subscribe(
        [&](Event&) { eventCount++; }, EventType::KeyPressed, EventCategory::Input);

    InputManager::GetInstance().SimulateKeyState(Keys::A, true); // real change → event
    InputManager::GetInstance().SimulateKeyState(Keys::A, true); // duplicate → no event
    InputManager::GetInstance().SimulateKeyState(Keys::A, true); // duplicate → no event
    EventHandler::GetInstance().ProcessEvents();

    CHECK(eventCount == 1);

    EventHandler::GetInstance().Unsubscribe(id);
}

// ---------------------------------------------------------------------------
// 3. SetKey deduplication — no event fired on redundant release
// ---------------------------------------------------------------------------

TEST_CASE("InputManager SetKey release deduplication") {
    InputFixture fix;

    int releaseCount = 0;
    size_t id = EventHandler::GetInstance().Subscribe(
        [&](Event&) { releaseCount++; }, EventType::KeyReleased, EventCategory::Input);

    // Press and advance a frame so the key is fully "held"
    InputManager::GetInstance().SimulateKeyState(Keys::Space, true);
    InputManager::GetInstance().SimulateUpdate();

    // Release once — real change
    InputManager::GetInstance().SimulateKeyState(Keys::Space, false);
    // Release again — duplicate, no new event
    InputManager::GetInstance().SimulateKeyState(Keys::Space, false);
    EventHandler::GetInstance().ProcessEvents();

    CHECK(releaseCount == 1);

    EventHandler::GetInstance().Unsubscribe(id);
}

// ---------------------------------------------------------------------------
// 4. GetKeyDown is only true for exactly one frame
// ---------------------------------------------------------------------------

TEST_CASE("InputManager GetKeyDown is single-frame") {
    InputFixture fix;

    InputManager& input = InputManager::GetInstance();

    // Frame 1: key goes down
    input.SimulateKeyState(Keys::W, true);
    CHECK(input.GetKeyDown(Keys::W));

    // Frame 2: Update() copies current → previous; key still held
    input.SimulateUpdate();
    input.SimulateKeyState(Keys::W, true);  // SetKey deduplicates, no change
    CHECK_FALSE(input.GetKeyDown(Keys::W)); // no longer "just pressed"
    CHECK(input.GetKey(Keys::W));           // but still held
}

// ---------------------------------------------------------------------------
// 5. GetKeyUp is only true for exactly one frame
// ---------------------------------------------------------------------------

TEST_CASE("InputManager GetKeyUp is single-frame") {
    InputFixture fix;

    InputManager& input = InputManager::GetInstance();

    // Press and hold for two frames
    input.SimulateKeyState(Keys::S, true);
    input.SimulateUpdate();

    // Release on frame 3
    input.SimulateKeyState(Keys::S, false);
    CHECK(input.GetKeyUp(Keys::S));

    // Frame 4: state has been advanced, key is simply not held
    input.SimulateUpdate();
    CHECK_FALSE(input.GetKeyUp(Keys::S));
    CHECK_FALSE(input.GetKey(Keys::S));
    CHECK_FALSE(input.GetKeyDown(Keys::S));
}

// ---------------------------------------------------------------------------
// 6. Multiple keys are tracked independently
// ---------------------------------------------------------------------------

TEST_CASE("InputManager independent key tracking") {
    InputFixture fix;

    InputManager& input = InputManager::GetInstance();

    input.SimulateKeyState(Keys::A, true);
    input.SimulateKeyState(Keys::B, true);
    input.SimulateUpdate();
    input.SimulateKeyState(Keys::A, false); // release A, keep B

    CHECK(input.GetKeyUp(Keys::A));
    CHECK_FALSE(input.GetKeyUp(Keys::B));

    CHECK(input.GetKey(Keys::B));
    CHECK_FALSE(input.GetKey(Keys::A));
}

// ---------------------------------------------------------------------------
// 7. Mouse button — GetMouseButtonDown is single-frame
// ---------------------------------------------------------------------------

TEST_CASE("InputManager GetMouseButtonDown is single-frame") {
    InputFixture fix;

    InputManager& input = InputManager::GetInstance();

    input.SimulateMouseButtonState(MouseButtons::BUTTON_RIGHT, true);
    CHECK(input.GetMouseButtonDown(MouseButtons::BUTTON_RIGHT));

    input.SimulateUpdate();
    // Still held but no longer "just down"
    CHECK_FALSE(input.GetMouseButtonDown(MouseButtons::BUTTON_RIGHT));
    CHECK(input.GetMouseButton(MouseButtons::BUTTON_RIGHT));
}

// ---------------------------------------------------------------------------
// 8. Mouse button — GetMouseButtonUp is single-frame
// ---------------------------------------------------------------------------

TEST_CASE("InputManager GetMouseButtonUp is single-frame") {
    InputFixture fix;

    InputManager& input = InputManager::GetInstance();

    input.SimulateMouseButtonState(MouseButtons::BUTTON_RIGHT, true);
    input.SimulateUpdate();
    input.SimulateMouseButtonState(MouseButtons::BUTTON_RIGHT, false);
    CHECK(input.GetMouseButtonUp(MouseButtons::BUTTON_RIGHT));

    input.SimulateUpdate();
    CHECK_FALSE(input.GetMouseButtonUp(MouseButtons::BUTTON_RIGHT));
}

// ---------------------------------------------------------------------------
// 9. SetMousePosition deduplication — no event when position unchanged
// ---------------------------------------------------------------------------

TEST_CASE("InputManager mouse position deduplication") {
    InputFixture fix;

    int moveEventCount = 0;
    size_t id = EventHandler::GetInstance().Subscribe(
        [&](Event&) { moveEventCount++; }, EventType::MouseMoved, EventCategory::Input);

    glm::vec2 pos(50.0f, 75.0f);
    InputManager::GetInstance().SimulateMousePosition(pos); // real change
    InputManager::GetInstance().SimulateMousePosition(pos); // duplicate
    InputManager::GetInstance().SimulateMousePosition(pos); // duplicate
    EventHandler::GetInstance().ProcessEvents();

    CHECK(moveEventCount == 1);

    EventHandler::GetInstance().Unsubscribe(id);
}

// ---------------------------------------------------------------------------
// 10. Mouse wheel always fires an event (no deduplication)
// ---------------------------------------------------------------------------

TEST_CASE("InputManager mouse wheel always fires event") {
    InputFixture fix;

    int scrollCount = 0;
    size_t id = EventHandler::GetInstance().Subscribe(
        [&](Event&) { scrollCount++; }, EventType::MouseScrolled, EventCategory::Input);

    InputManager::GetInstance().SimulateMouseWheel(1.0f);
    InputManager::GetInstance().SimulateMouseWheel(1.0f); // same value, but still fires
    InputManager::GetInstance().SimulateMouseWheel(-1.0f);
    EventHandler::GetInstance().ProcessEvents();

    CHECK(scrollCount == 3);

    EventHandler::GetInstance().Unsubscribe(id);
}

// ---------------------------------------------------------------------------
// 11. Update() fires KeyIsPressed for every currently-held key
// ---------------------------------------------------------------------------

TEST_CASE("InputManager Update fires KeyIsPressed events for held keys") {
    InputFixture fix;

    int isHeldCount = 0;
    size_t id = EventHandler::GetInstance().Subscribe(
        [&](Event&) { isHeldCount++; }, EventType::KeyIsPressed, EventCategory::Input);

    // Hold two keys for two full frames
    InputManager::GetInstance().SimulateKeyState(Keys::A, true);
    InputManager::GetInstance().SimulateKeyState(Keys::D, true);
    InputManager::GetInstance().SimulateUpdate(); // frame 1: both now
    InputManager::GetInstance().SimulateKeyState(Keys::A, true);
    InputManager::GetInstance().SimulateKeyState(Keys::D, true);
    InputManager::GetInstance().SimulateUpdate(); // frame 2: both now in previous too
                                                  //
    EventHandler::GetInstance().ProcessEvents();

    // Update() iterates all keys — 2 held × 1 Update() call = 2 events
    CHECK(isHeldCount == 2);

    // Second frame: still held → 2 more events
    isHeldCount = 0;
    InputManager::GetInstance().SimulateUpdate();
    EventHandler::GetInstance().ProcessEvents();
    CHECK(isHeldCount == 2);

    EventHandler::GetInstance().Unsubscribe(id);
}

// ---------------------------------------------------------------------------
// 12. Update() does NOT fire KeyIsPressed for keys that are only in current
//     (i.e. just pressed this frame, not yet in previous)
// ---------------------------------------------------------------------------

TEST_CASE("InputManager Update does not fire KeyIsPressed on frame of press") {
    InputFixture fix;

    int isHeldCount = 0;
    size_t id = EventHandler::GetInstance().Subscribe(
        [&](Event&) { isHeldCount++; }, EventType::KeyIsPressed, EventCategory::Input);

    // Press but do NOT call SimulateUpdate — key is only in "current"
    InputManager::GetInstance().SimulateKeyState(Keys::E, true);
    // Call Update() to fire events (but GetKeyUnsafe checks current AND previous)
    InputManager::GetInstance().SimulateUpdate();
    EventHandler::GetInstance().ProcessEvents();

    CHECK(isHeldCount == 0); // not yet in "previous", so GetKeyUnsafe returns false

    EventHandler::GetInstance().Unsubscribe(id);
}

// ---------------------------------------------------------------------------
// 13. SimulateReset clears all state — nothing fires after reset
// ---------------------------------------------------------------------------

TEST_CASE("InputManager SimulateReset clears all state") {
    InputFixture fix;

    InputManager& input = InputManager::GetInstance();

    // Build up some state
    input.SimulateKeyState(Keys::A, true);
    input.SimulateMouseButtonState(MouseButtons::BUTTON_LEFT, true);
    input.SimulateMousePosition({100.0f, 200.0f});
    input.SimulateUpdate();

    // Now wipe it all
    input.SimulateReset();

    CHECK_FALSE(input.GetKey(Keys::A));
    CHECK_FALSE(input.GetKeyDown(Keys::A));
    CHECK_FALSE(input.GetKeyUp(Keys::A));

    CHECK_FALSE(input.GetMouseButton(MouseButtons::BUTTON_LEFT));
    CHECK_FALSE(input.GetMouseButtonDown(MouseButtons::BUTTON_LEFT));
    CHECK_FALSE(input.GetMouseButtonUp(MouseButtons::BUTTON_LEFT));

    CHECK(input.GetMousePosition() == glm::vec2(0.0f, 0.0f));
}

// ---------------------------------------------------------------------------
// 14. Press → release → re-press cycle produces correct events each frame
// ---------------------------------------------------------------------------

TEST_CASE("InputManager press-release-press cycle") {
    InputFixture fix;

    InputManager& input = InputManager::GetInstance();

    // Frame 1: press
    input.SimulateKeyState(Keys::Space, true);
    CHECK(input.GetKeyDown(Keys::Space));
    CHECK_FALSE(input.GetKey(Keys::Space));
    input.SimulateUpdate();

    // Frame 2: still held
    CHECK_FALSE(input.GetKeyDown(Keys::Space));
    CHECK(input.GetKey(Keys::Space));
    CHECK_FALSE(input.GetKeyDown(Keys::Space));
    input.SimulateUpdate();

    // Frame 3: release
    input.SimulateKeyState(Keys::Space, false);
    CHECK(input.GetKeyUp(Keys::Space));
    CHECK_FALSE(input.GetKey(Keys::Space));
    input.SimulateUpdate();

    // Frame 4: idle
    CHECK_FALSE(input.GetKeyUp(Keys::Space));
    CHECK_FALSE(input.GetKey(Keys::Space));
    CHECK_FALSE(input.GetKeyDown(Keys::Space));
    input.SimulateUpdate();

    // Frame 5: re-press
    input.SimulateKeyState(Keys::Space, true);
    CHECK(input.GetKeyDown(Keys::Space));
    CHECK_FALSE(input.GetKey(Keys::Space));
}

// ---------------------------------------------------------------------------
// 15. Mouse position event carries correct coordinates
// ---------------------------------------------------------------------------

TEST_CASE("InputManager mouse moved event carries correct coordinates") {
    InputFixture fix;

    glm::vec2 captured(-1.0f, -1.0f);
    size_t id = EventHandler::GetInstance().Subscribe(
        [&](Event& e) {
            auto& data = std::get<std::array<u16, 8>>(e.GetContext().raw_data);
            captured = {static_cast<float>(data.at(0)), static_cast<float>(data.at(1))};
        },
        EventType::MouseMoved,
        EventCategory::Input);

    InputManager::GetInstance().SimulateMousePosition({320.0f, 240.0f});
    EventHandler::GetInstance().ProcessEvents();

    CHECK(captured.x == doctest::Approx(320.0f));
    CHECK(captured.y == doctest::Approx(240.0f));

    EventHandler::GetInstance().Unsubscribe(id);
}
