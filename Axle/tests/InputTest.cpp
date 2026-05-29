#include <doctest.h>
#include <glm/vec2.hpp>

#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Layer/Layer.hpp"
#include "Core/Layer/LayerStack.hpp"
#include "Core/Logger/Log.hpp"

using namespace Axle;

// ─── Spy layer ────────────────────────────────────────────────────────────────
//
// Receives events and dispatches them via EventDispatcher, mirroring the
// real OnEvent() pattern. The predicate receives the already-downcast
// concrete event type via the dispatcher callback.

class InputSpyLayer : public Layer {
public:
    using RawPredicate = std::function<void(Event&)>;

    int eventCount = 0;

    explicit InputSpyLayer(const std::string& name = "InputSpyLayer", RawPredicate pred = nullptr)
        : Layer(name),
          m_Pred(std::move(pred)) {}

    void OnAttach() override {}
    void OnDettach() override {}
    void OnAttachRender() override {}
    void OnDettachRender() override {}
    void OnUpdate(f64) override {}
    void OnRender(f64) override {}

    void OnEvent(Event& event) override {
        eventCount++;
        if (m_Pred)
            m_Pred(event);
    }

private:
    RawPredicate m_Pred;
};

// ─── Fixture ──────────────────────────────────────────────────────────────────

struct InputFixture {
    LayerStack stack;

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

    void process() {
        EventHandler::GetInstance().ProcessEvents(stack.rbegin(), stack.rend());
    }
};

// ─── Key handling ─────────────────────────────────────────────────────────────

TEST_CASE("Input system key handling") {
    InputFixture f;

    // Spy verifies that key events carry the correct key via EventDispatcher
    InputSpyLayer* spy = new InputSpyLayer("KeySpy", [](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
            CHECK(e.GetEventCategory() == EventCategory::Input);
            CHECK(e.GetKey() == Keys::A);
            return true;
        });
        dispatcher.Dispatch<KeyReleasedEvent>([](KeyReleasedEvent& e) {
            CHECK(e.GetEventCategory() == EventCategory::Input);
            CHECK(e.GetKey() == Keys::A);
            return true;
        });
        dispatcher.Dispatch<KeyIsPressedEvent>([](KeyIsPressedEvent& e) {
            CHECK(e.GetEventCategory() == EventCategory::Input);
            CHECK(e.GetKey() == Keys::A);
            return true;
        });
    });
    f.stack.PushLayer(spy);

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

        f.process();
    }

    SUBCASE("Key Up") {
        InputManager::GetInstance().SimulateKeyState(Keys::A, true);
        f.process();
        InputManager::GetInstance().SimulateUpdate();
        InputManager::GetInstance().SimulateKeyState(Keys::A, false);
        f.process();

        CHECK(InputManager::GetInstance().GetKeyUp(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKeyDown(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKey(Keys::A));
    }

    SUBCASE("Key Is Pressed") {
        InputManager::GetInstance().SimulateKeyState(Keys::A, true);
        f.process();

        CHECK_FALSE(InputManager::GetInstance().GetKey(Keys::A));

        InputManager::GetInstance().SimulateUpdate();
        InputManager::GetInstance().SimulateKeyState(Keys::A, true);
        f.process();

        CHECK(InputManager::GetInstance().GetKey(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKeyUp(Keys::A));
        CHECK_FALSE(InputManager::GetInstance().GetKeyDown(Keys::A));
    }
}

// ─── Mouse button handling ────────────────────────────────────────────────────

TEST_CASE("Input system mouse button handling") {
    InputFixture f;

    InputSpyLayer* spy = new InputSpyLayer("MouseButtonSpy", [](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>([](MouseButtonPressedEvent& e) {
            CHECK(e.GetEventCategory() == EventCategory::Input);
            CHECK(e.GetMouseButton() == MouseButtons::Left);
            return true;
        });
        dispatcher.Dispatch<MouseButtonReleasedEvent>([](MouseButtonReleasedEvent& e) {
            CHECK(e.GetEventCategory() == EventCategory::Input);
            CHECK(e.GetMouseButton() == MouseButtons::Left);
            return true;
        });
        dispatcher.Dispatch<MouseButtonIsPressedEvent>([](MouseButtonIsPressedEvent& e) {
            CHECK(e.GetEventCategory() == EventCategory::Input);
            CHECK(e.GetMouseButton() == MouseButtons::Left);
            return true;
        });
    });
    f.stack.PushLayer(spy);

    SUBCASE("No mouse button action") {
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonDown(MouseButtons::Left));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonUp(MouseButtons::Left));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButton(MouseButtons::Left));
    }

    SUBCASE("Mouse button Down") {
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::Left, true);
        f.process();

        CHECK(InputManager::GetInstance().GetMouseButtonDown(MouseButtons::Left));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonUp(MouseButtons::Left));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButton(MouseButtons::Left));
    }

    SUBCASE("Mouse button Up") {
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::Left, true);
        f.process();
        InputManager::GetInstance().SimulateUpdate();
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::Left, false);
        f.process();

        CHECK(InputManager::GetInstance().GetMouseButtonUp(MouseButtons::Left));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonDown(MouseButtons::Left));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButton(MouseButtons::Left));
    }

    SUBCASE("Mouse button Is Pressed") {
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::Left, true);
        f.process();

        CHECK_FALSE(InputManager::GetInstance().GetMouseButton(MouseButtons::Left));

        InputManager::GetInstance().SimulateUpdate();
        InputManager::GetInstance().SimulateMouseButtonState(MouseButtons::Left, true);
        f.process();

        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonUp(MouseButtons::Left));
        CHECK_FALSE(InputManager::GetInstance().GetMouseButtonDown(MouseButtons::Left));
        CHECK(InputManager::GetInstance().GetMouseButton(MouseButtons::Left));
    }
}

// ─── Mouse position handling ──────────────────────────────────────────────────

TEST_CASE("Mouse position handling") {
    InputFixture f;

    glm::vec2 testPosition(100.0f, 200.0f);
    InputManager::GetInstance().SimulateMousePosition(testPosition);

    SUBCASE("Get Mouse Position") {
        CHECK(InputManager::GetInstance().GetMousePosition() == testPosition);
    }

    SUBCASE("Set Mouse Position") {
        glm::vec2 newPosition(300.0f, 400.0f);
        InputManager::GetInstance().SimulateMousePosition(newPosition);
        CHECK(InputManager::GetInstance().GetMousePosition() == newPosition);
    }
}

// ─── Mouse wheel handling ─────────────────────────────────────────────────────

TEST_CASE("Mouse wheel handling") {
    InputFixture f;

    f32 receivedY = 0.0f;
    InputSpyLayer* spy = new InputSpyLayer("WheelSpy", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseScrollEvent>([&](MouseScrollEvent& e) {
            CHECK(e.GetEventCategory() == EventCategory::Input);
            receivedY = e.GetYOffset();
            return true;
        });
    });
    f.stack.PushLayer(spy);

    InputManager::GetInstance().SimulateMouseWheel(1.0f, 1.0f);
    InputManager::GetInstance().SimulateUpdate();
    f.process();

    CHECK(receivedY == doctest::Approx(1.0f));
    CHECK(spy->eventCount >= 1);
}

// ─── Singleton lifecycle ──────────────────────────────────────────────────────

TEST_CASE("InputManager singleton lifecycle") {
    Log::Init();
    EventHandler::Init();

    InputManager::Init();
    InputManager& first = InputManager::GetInstance();
    InputManager::Init(); // second call should warn and be a no-op
    InputManager& second = InputManager::GetInstance();
    CHECK(&first == &second);

    InputManager::ShutDown();
    InputManager::Init();
    CHECK_NOTHROW(InputManager::GetInstance());
    InputManager::ShutDown();

    EventHandler::ShutDown();
}

// ─── SetKey deduplication ─────────────────────────────────────────────────────

TEST_CASE("InputManager SetKey deduplication") {
    InputFixture f;

    int pressCount = 0;
    InputSpyLayer* spy = new InputSpyLayer("DedupSpy", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent&) {
            pressCount++;
            return true;
        });
    });
    f.stack.PushLayer(spy);

    InputManager::GetInstance().SimulateKeyState(Keys::A, true); // real change → event
    InputManager::GetInstance().SimulateKeyState(Keys::A, true); // duplicate → no event
    InputManager::GetInstance().SimulateKeyState(Keys::A, true); // duplicate → no event
    f.process();

    CHECK(pressCount == 1);
}

TEST_CASE("InputManager SetKey release deduplication") {
    InputFixture f;

    int releaseCount = 0;
    InputSpyLayer* spy = new InputSpyLayer("ReleaseDedupSpy", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent&) {
            releaseCount++;
            return true;
        });
    });
    f.stack.PushLayer(spy);

    InputManager::GetInstance().SimulateKeyState(Keys::Space, true);
    InputManager::GetInstance().SimulateUpdate();
    InputManager::GetInstance().SimulateKeyState(Keys::Space, false); // real change
    InputManager::GetInstance().SimulateKeyState(Keys::Space, false); // duplicate
    f.process();

    CHECK(releaseCount == 1);
}

// ─── Single-frame semantics ───────────────────────────────────────────────────

TEST_CASE("InputManager GetKeyDown is single-frame") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    input.SimulateKeyState(Keys::W, true);
    CHECK(input.GetKeyDown(Keys::W));

    input.SimulateUpdate();
    input.SimulateKeyState(Keys::W, true); // deduplicated
    CHECK_FALSE(input.GetKeyDown(Keys::W));
    CHECK(input.GetKey(Keys::W));
}

TEST_CASE("InputManager GetKeyUp is single-frame") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    input.SimulateKeyState(Keys::S, true);
    input.SimulateUpdate();
    input.SimulateKeyState(Keys::S, false);
    CHECK(input.GetKeyUp(Keys::S));

    input.SimulateUpdate();
    CHECK_FALSE(input.GetKeyUp(Keys::S));
    CHECK_FALSE(input.GetKey(Keys::S));
    CHECK_FALSE(input.GetKeyDown(Keys::S));
}

TEST_CASE("InputManager independent key tracking") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    input.SimulateKeyState(Keys::A, true);
    input.SimulateKeyState(Keys::B, true);
    input.SimulateUpdate();
    input.SimulateKeyState(Keys::A, false);

    CHECK(input.GetKeyUp(Keys::A));
    CHECK_FALSE(input.GetKeyUp(Keys::B));
    CHECK(input.GetKey(Keys::B));
    CHECK_FALSE(input.GetKey(Keys::A));
}

TEST_CASE("InputManager GetMouseButtonDown is single-frame") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    input.SimulateMouseButtonState(MouseButtons::Right, true);
    CHECK(input.GetMouseButtonDown(MouseButtons::Right));

    input.SimulateUpdate();
    CHECK_FALSE(input.GetMouseButtonDown(MouseButtons::Right));
    CHECK(input.GetMouseButton(MouseButtons::Right));
}

TEST_CASE("InputManager GetMouseButtonUp is single-frame") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    input.SimulateMouseButtonState(MouseButtons::Right, true);
    input.SimulateUpdate();
    input.SimulateMouseButtonState(MouseButtons::Right, false);
    CHECK(input.GetMouseButtonUp(MouseButtons::Right));

    input.SimulateUpdate();
    CHECK_FALSE(input.GetMouseButtonUp(MouseButtons::Right));
}

// ─── Mouse position deduplication ────────────────────────────────────────────

TEST_CASE("InputManager mouse position deduplication") {
    InputFixture f;

    int moveCount = 0;
    InputSpyLayer* spy = new InputSpyLayer("MoveSpy", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent&) {
            moveCount++;
            return true;
        });
    });
    f.stack.PushLayer(spy);

    glm::vec2 pos(50.0f, 75.0f);
    InputManager::GetInstance().SimulateMousePosition(pos); // real change
    InputManager::GetInstance().SimulateMousePosition(pos); // duplicate
    InputManager::GetInstance().SimulateMousePosition(pos); // duplicate
    f.process();

    CHECK(moveCount == 1);
}

// ─── Mouse moved event carries correct coordinates ────────────────────────────

TEST_CASE("InputManager mouse moved event carries correct coordinates") {
    InputFixture f;

    f32 capturedX = -1.0f, capturedY = -1.0f;
    InputSpyLayer* spy = new InputSpyLayer("CoordSpy", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) {
            capturedX = e.GetX();
            capturedY = e.GetY();
            return true;
        });
    });
    f.stack.PushLayer(spy);

    InputManager::GetInstance().SimulateMousePosition({320.0f, 240.0f});
    f.process();

    CHECK(capturedX == doctest::Approx(320.0f));
    CHECK(capturedY == doctest::Approx(240.0f));
}

// ─── Mouse wheel always fires ─────────────────────────────────────────────────

TEST_CASE("InputManager mouse wheel always fires event") {
    InputFixture f;

    int scrollCount = 0;
    InputSpyLayer* spy = new InputSpyLayer("ScrollSpy", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseScrollEvent>([&](MouseScrollEvent&) {
            scrollCount++;
            return true;
        });
    });
    f.stack.PushLayer(spy);

    InputManager::GetInstance().SimulateMouseWheel(1.0f, 1.0f);
    InputManager::GetInstance().SimulateMouseWheel(1.0f, 1.0f);
    InputManager::GetInstance().SimulateMouseWheel(-1.0f, -1.0f);
    f.process();

    CHECK(scrollCount == 3);
}

// ─── Mouse scroll event carries correct offset ────────────────────────────────

TEST_CASE("InputManager mouse scroll event carries correct offset") {
    InputFixture f;

    f32 capturedOffset = 0.0f;
    InputSpyLayer* spy = new InputSpyLayer("ScrollOffsetSpy", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseScrollEvent>([&](MouseScrollEvent& e) {
            capturedOffset = e.GetYOffset();
            return true;
        });
    });
    f.stack.PushLayer(spy);

    InputManager::GetInstance().SimulateMouseWheel(-2.5f, -2.5f);
    f.process();

    CHECK(capturedOffset == doctest::Approx(-2.5f));
}

// ─── Update() / KeyIsPressed semantics ───────────────────────────────────────

TEST_CASE("InputManager Update fires KeyIsPressed events for held keys") {
    InputFixture f;

    int isHeldCount = 0;
    InputSpyLayer* spy = new InputSpyLayer("HeldSpy", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyIsPressedEvent>([&](KeyIsPressedEvent&) {
            isHeldCount++;
            return true;
        });
    });
    f.stack.PushLayer(spy);

    InputManager::GetInstance().SimulateKeyState(Keys::A, true);
    InputManager::GetInstance().SimulateKeyState(Keys::D, true);
    InputManager::GetInstance().SimulateUpdate(); // frame 1
    InputManager::GetInstance().SimulateKeyState(Keys::A, true);
    InputManager::GetInstance().SimulateKeyState(Keys::D, true);
    InputManager::GetInstance().SimulateUpdate(); // frame 2
    f.process();

    CHECK(isHeldCount == 2);

    isHeldCount = 0;
    InputManager::GetInstance().SimulateUpdate();
    f.process();
    CHECK(isHeldCount == 2);
}

TEST_CASE("InputManager Update does not fire KeyIsPressed on frame of press") {
    InputFixture f;

    int isHeldCount = 0;
    InputSpyLayer* spy = new InputSpyLayer("HeldSpyFirstFrame", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyIsPressedEvent>([&](KeyIsPressedEvent&) {
            isHeldCount++;
            return true;
        });
    });
    f.stack.PushLayer(spy);

    InputManager::GetInstance().SimulateKeyState(Keys::E, true);
    InputManager::GetInstance().SimulateUpdate();
    f.process();

    CHECK(isHeldCount == 0);
}

// ─── SimulateReset ────────────────────────────────────────────────────────────

TEST_CASE("InputManager SimulateReset clears all state") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    input.SimulateKeyState(Keys::A, true);
    input.SimulateMouseButtonState(MouseButtons::Left, true);
    input.SimulateMousePosition({100.0f, 200.0f});
    input.SimulateUpdate();
    input.SimulateReset();

    CHECK_FALSE(input.GetKey(Keys::A));
    CHECK_FALSE(input.GetKeyDown(Keys::A));
    CHECK_FALSE(input.GetKeyUp(Keys::A));
    CHECK_FALSE(input.GetMouseButton(MouseButtons::Left));
    CHECK_FALSE(input.GetMouseButtonDown(MouseButtons::Left));
    CHECK_FALSE(input.GetMouseButtonUp(MouseButtons::Left));
    CHECK(input.GetMousePosition() == glm::vec2(0.0f, 0.0f));
}

// ─── Press-release-press cycle ────────────────────────────────────────────────

TEST_CASE("InputManager press-release-press cycle") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    // Frame 1: press
    input.SimulateKeyState(Keys::Space, true);
    CHECK(input.GetKeyDown(Keys::Space));
    CHECK_FALSE(input.GetKey(Keys::Space));
    input.SimulateUpdate();

    // Frame 2: held
    CHECK_FALSE(input.GetKeyDown(Keys::Space));
    CHECK(input.GetKey(Keys::Space));
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

// ─── Concurrency ──────────────────────────────────────────────────────────────

TEST_CASE("InputManager concurrent reads do not deadlock") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    input.SimulateKeyState(Keys::A, true);
    input.SimulateUpdate();
    input.SimulateKeyState(Keys::A, true);

    constexpr int kThreads = 8;
    constexpr int kReads = 1000;

    std::atomic<int> trueCount{0};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kReads; ++i) {
                if (input.GetKey(Keys::A))
                    trueCount++;
                if (input.GetKeyDown(Keys::A))
                    trueCount++;
                if (input.GetMousePosition().x >= 0)
                    trueCount++;
            }
        });
    }
    for (auto& t : threads)
        t.join();

    CHECK(trueCount == kThreads * kReads * 2);
}

TEST_CASE("InputManager concurrent writes do not corrupt state") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    constexpr int kThreads = 8;
    constexpr int kOps = 500;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < kOps; ++i) {
                bool pressed = (i % 2 == 0);
                input.SimulateKeyState(Keys::A, pressed);
                input.SimulateMouseButtonState(MouseButtons::Left, pressed);
            }
        });
    }
    for (auto& t : threads)
        t.join();

    CHECK_NOTHROW(input.GetKey(Keys::A));
    CHECK_NOTHROW(input.GetMouseButton(MouseButtons::Left));
}

TEST_CASE("InputManager concurrent read-write does not deadlock") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    constexpr int kWriters = 4;
    constexpr int kReaders = 4;
    constexpr int kOps = 300;

    std::vector<std::thread> threads;
    threads.reserve(kWriters + kReaders);

    for (int t = 0; t < kWriters; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < kOps; ++i) {
                input.SimulateKeyState(Keys::W, i % 2 == 0);
                input.SimulateMouseButtonState(MouseButtons::Right, i % 2 == 0);
                input.SimulateMousePosition({static_cast<float>(i), static_cast<float>(t)});
            }
        });
    }
    for (int t = 0; t < kReaders; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kOps; ++i) {
                (void) input.GetKey(Keys::W);
                (void) input.GetKeyDown(Keys::W);
                (void) input.GetKeyUp(Keys::W);
                (void) input.GetMouseButton(MouseButtons::Right);
                (void) input.GetMousePosition();
            }
        });
    }
    for (auto& t : threads)
        t.join();

    CHECK_NOTHROW(input.GetKey(Keys::W));
}

TEST_CASE("InputManager Update is safe under concurrent writes") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    input.SimulateKeyState(Keys::A, true);
    input.SimulateKeyState(Keys::D, true);
    input.SimulateUpdate();

    constexpr int kWriters = 4;
    constexpr int kIter = 200;

    std::vector<std::thread> writers;
    writers.reserve(kWriters);
    for (int t = 0; t < kWriters; ++t) {
        writers.emplace_back([&, t]() {
            for (int i = 0; i < kIter; ++i) {
                input.SimulateKeyState(Keys::A, i % 2 == 0);
                input.SimulateMouseButtonState(MouseButtons::Left, i % 3 == 0);
            }
        });
    }
    for (int i = 0; i < kIter; ++i)
        input.SimulateUpdate();

    for (auto& w : writers)
        w.join();

    CHECK_NOTHROW(input.GetKey(Keys::A));
}

TEST_CASE("InputManager rapid press-release cycle from multiple threads") {
    InputFixture f;
    InputManager& input = InputManager::GetInstance();

    constexpr int kCycles = 500;

    std::thread t1([&]() {
        for (int i = 0; i < kCycles; ++i) {
            input.SimulateKeyState(Keys::A, true);
            input.SimulateKeyState(Keys::A, false);
        }
    });
    std::thread t2([&]() {
        for (int i = 0; i < kCycles; ++i) {
            input.SimulateKeyState(Keys::B, true);
            input.SimulateKeyState(Keys::B, false);
        }
    });
    t1.join();
    t2.join();

    bool aDown = input.GetKeyDown(Keys::A);
    bool bDown = input.GetKeyDown(Keys::B);
    bool aHeld = input.GetKey(Keys::A);
    bool bHeld = input.GetKey(Keys::B);

    CHECK_FALSE((aDown && aHeld));
    CHECK_FALSE((bDown && bHeld));
}
