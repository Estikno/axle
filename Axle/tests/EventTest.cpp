#include <doctest.h>

#include "Core/Input/InputState.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Layer/Layer.hpp"
#include "Core/Layer/LayerStack.hpp"

using namespace Axle;

// ─── Helpers ────────────────────────────────────────────────────────────────
class TestKeyPressedEvent : public Event {
public:
    explicit TestKeyPressedEvent(Keys key)
        : m_Key(key) {}
    Keys GetKey() const noexcept {
        return m_Key;
    }

    DEFINE_EVENT_TYPE(KeyPressed);
    DEFINE_EVENT_CATEGORY(Input);

private:
    Keys m_Key;
};

class TestWindowResizeEvent : public Event {
public:
    TestWindowResizeEvent(u32 w, u32 h)
        : m_W(w),
          m_H(h) {}
    u32 GetWidth() const noexcept {
        return m_W;
    }
    u32 GetHeight() const noexcept {
        return m_H;
    }

    DEFINE_EVENT_TYPE(WindowResize);
    DEFINE_EVENT_CATEGORY(Window);

private:
    u32 m_W, m_H;
};

/**
 * Minimal concrete Layer that records every event it receives.
 * The constructor accepts an optional predicate so individual test cases
 * can inject custom assertions without subclassing again.
 */
class TestLayer : public Layer {
public:
    using EventPredicate = std::function<void(Event&)>;

    int eventCount = 0;

    explicit TestLayer(const std::string& name = "TestLayer", EventPredicate pred = nullptr)
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
    EventPredicate m_Pred;
};

/**
 * A layer that immediately marks every event as handled, simulating a
 * consumer that blocks propagation.
 */
class ConsumingLayer : public Layer {
public:
    int eventCount = 0;

    explicit ConsumingLayer(const std::string& name = "ConsumingLayer")
        : Layer(name) {}

    void OnAttach() override {}
    void OnDettach() override {}
    void OnAttachRender() override {}
    void OnDettachRender() override {}
    void OnUpdate(f64) override {}
    void OnRender(f64) override {}

    void OnEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        // Dispatch returns true and marks handled=true when the type matches
        bool dispatched = dispatcher.Dispatch<TestKeyPressedEvent>([this](TestKeyPressedEvent&) {
            eventCount++;
            return true; // true → Handle(true) → stops propagation
        });

        // Also count non-AppRender events so tests can inspect them if needed
        if (!dispatched)
            eventCount++;
    }
};

struct EHFixture {
    EHFixture() {
        Log::Init();
        EventHandler::Init();
    }
    ~EHFixture() {
        EventHandler::ShutDown();
    }
    void process(LayerStack& stack) {
        EventHandler::GetInstance().ProcessEvents(stack.rbegin(), stack.rend());
    }
};

// ─── EventDispatcher unit tests ───────────────────────────────────────────────
TEST_CASE("EventDispatcher dispatches to matching type") {
    TestKeyPressedEvent e(Keys::A);
    EventDispatcher dispatcher(e);

    bool called = false;
    bool result = dispatcher.Dispatch<TestKeyPressedEvent>([&](TestKeyPressedEvent&) {
        called = true;
        return true;
    });

    CHECK(result == true);
    CHECK(called == true);
    CHECK(e.IsHandled() == true);
}

TEST_CASE("EventDispatcher does not dispatch to non-matching type") {
    TestWindowResizeEvent e(1, 1);
    EventDispatcher dispatcher(e);

    bool called = false;
    bool result = dispatcher.Dispatch<TestKeyPressedEvent>([&](TestKeyPressedEvent&) {
        called = true;
        return true;
    });

    CHECK(result == false);
    CHECK(called == false);
    CHECK(e.IsHandled() == false);
}

TEST_CASE("EventDispatcher Handle(false) leaves event unhandled") {
    TestKeyPressedEvent e(Keys::A);
    EventDispatcher dispatcher(e);

    dispatcher.Dispatch<TestKeyPressedEvent>([](TestKeyPressedEvent&) {
        return false; // handler signals "not fully consumed"
    });

    CHECK(e.IsHandled() == false);
}

TEST_CASE("EventDispatcher multiple dispatches on same event — first match wins") {
    TestKeyPressedEvent e(Keys::A);
    EventDispatcher dispatcher(e);

    int callCount = 0;

    dispatcher.Dispatch<TestKeyPressedEvent>([&](TestKeyPressedEvent&) {
        callCount++;
        return true;
    });
    // Dispatching again: the event is already handled but Dispatch itself
    // still calls the handler if the type matches — Handle(true) is idempotent.
    dispatcher.Dispatch<TestKeyPressedEvent>([&](TestKeyPressedEvent&) {
        callCount++;
        return true;
    });

    // Both matched and were called — handling is the caller's responsibility
    CHECK(callCount == 2);
    CHECK(e.IsHandled() == true);
}

TEST_CASE("EventDispatcher downcasts correctly — payload is accessible") {
    TestWindowResizeEvent e(1920, 1080);
    EventDispatcher dispatcher(e);

    u32 w = 0, h = 0;
    dispatcher.Dispatch<TestWindowResizeEvent>([&](TestWindowResizeEvent& ev) {
        w = ev.GetWidth();
        h = ev.GetHeight();
        return true;
    });

    CHECK(w == 1920);
    CHECK(h == 1080);
}

TEST_CASE("EventDispatcher with KeyPressedEvent carries key payload") {
    TestKeyPressedEvent e(Keys::A);
    EventDispatcher dispatcher(e);

    Keys received = Keys::Unknown;
    dispatcher.Dispatch<TestKeyPressedEvent>([&](TestKeyPressedEvent& ev) {
        received = ev.GetKey();
        return true;
    });

    CHECK(received == Keys::A);
}

// ─── Core dispatch through layer stack ───────────────────────────────────────

TEST_CASE("Single layer receives dispatched event") {
    EHFixture f;
    LayerStack stack;

    TestLayer* layer = new TestLayer("L1", [](Event& e) {
        CHECK(e.GetEventType() == EventType::KeyPressed);
        CHECK(e.GetEventCategory() == EventCategory::Input);
    });
    stack.PushLayer(layer);

    TestKeyPressedEvent e(Keys::A);
    CHECK_NOTHROW(AX_SUBMIT_EVENT(e));
    f.process(stack);

    CHECK(layer->eventCount == 1);
}

TEST_CASE("Multiple layers each receive the event") {
    EHFixture f;
    LayerStack stack;

    TestLayer* l1 = new TestLayer("L1");
    TestLayer* l2 = new TestLayer("L2");
    TestLayer* l3 = new TestLayer("L3");
    stack.PushLayer(l1);
    stack.PushLayer(l2);
    stack.PushLayer(l3);

    TestWindowResizeEvent e(1, 1);
    AX_SUBMIT_EVENT(e);
    f.process(stack);

    CHECK(l1->eventCount == 1);
    CHECK(l2->eventCount == 1);
    CHECK(l3->eventCount == 1);
}

TEST_CASE("Concrete event subclass is dispatched and downcast correctly via layer") {
    EHFixture f;
    LayerStack stack;

    Keys receivedKey = Keys::Unknown;
    TestLayer* layer = new TestLayer("L1", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<TestKeyPressedEvent>([&](TestKeyPressedEvent& e) {
            receivedKey = e.GetKey();
            return true;
        });
    });
    stack.PushLayer(layer);

    TestKeyPressedEvent e(Keys::Space);
    AX_SUBMIT_EVENT(e);
    f.process(stack);

    CHECK(layer->eventCount == 1);
    CHECK(receivedKey == Keys::Space);
}

TEST_CASE("WindowResizeEvent payload survives the queue and is accessible in layer") {
    EHFixture f;
    LayerStack stack;

    u32 gotW = 0, gotH = 0;
    TestLayer* layer = new TestLayer("L1", [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<TestWindowResizeEvent>([&](TestWindowResizeEvent& e) {
            gotW = e.GetWidth();
            gotH = e.GetHeight();
            return true;
        });
    });
    stack.PushLayer(layer);

    TestWindowResizeEvent e(1280, 720);
    AX_SUBMIT_EVENT(e);
    f.process(stack);

    CHECK(gotW == 1280);
    CHECK(gotH == 720);
}

// ─── Propagation ─────────────────────────────────────────────────────────────

TEST_CASE("Handled event stops propagation through layer stack") {
    EHFixture f;
    LayerStack stack;

    // ConsumingLayer handles AppRenderEvent and returns true → IsHandled()
    // EventHandler::Notify() checks IsHandled() and breaks the loop.
    TestLayer* bottom = new TestLayer("Bottom");
    ConsumingLayer* top = new ConsumingLayer("Top");
    stack.PushLayer(bottom);
    stack.PushOverlay(top); // overlay is visited first (rbegin → rend)

    TestKeyPressedEvent e(Keys::K);
    AX_SUBMIT_EVENT(e);
    f.process(stack);

    CHECK(top->eventCount == 1);
    CHECK(bottom->eventCount == 0);
}

TEST_CASE("Unhandled event reaches all layers") {
    EHFixture f;
    LayerStack stack;

    TestLayer* l1 = new TestLayer("L1");
    TestLayer* l2 = new TestLayer("L2");
    stack.PushLayer(l1);
    stack.PushLayer(l2);

    TestKeyPressedEvent e(Keys::A);
    AX_SUBMIT_EVENT(e);
    f.process(stack);

    CHECK(l1->eventCount == 1);
    CHECK(l2->eventCount == 1);
}

TEST_CASE("Dispatcher returning false leaves event unhandled and propagation continues") {
    EHFixture f;
    LayerStack stack;

    // Both layers dispatch but return false — event must reach both
    int callCount = 0;
    auto pred = [&](Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<TestKeyPressedEvent>([](TestKeyPressedEvent&) {
            return false; // not fully consumed
        });
        callCount++;
    };

    TestLayer* l1 = new TestLayer("L1", pred);
    TestLayer* l2 = new TestLayer("L2", pred);
    stack.PushLayer(l1);
    stack.PushLayer(l2);

    TestKeyPressedEvent e(Keys::K);
    AX_SUBMIT_EVENT(e);
    f.process(stack);

    CHECK(callCount == 2);
}

// ─── Queue semantics ─────────────────────────────────────────────────────────

TEST_CASE("Queue is drained after ProcessEvents") {
    EHFixture f;
    LayerStack stack;
    TestLayer* layer = new TestLayer();
    stack.PushLayer(layer);

    TestWindowResizeEvent e(0, 0);
    AX_SUBMIT_EVENT(e);
    f.process(stack); // drains queue
    f.process(stack); // nothing left

    CHECK(layer->eventCount == 1);
}

TEST_CASE("Multiple events are processed in dispatch order") {
    EHFixture f;
    LayerStack stack;

    std::vector<EventType> received;
    TestLayer* layer = new TestLayer("L1", [&](Event& e) { received.push_back(e.GetEventType()); });
    stack.PushLayer(layer);

    for (int i = 0; i < 5; ++i)
        AX_SUBMIT_EVENT(TestKeyPressedEvent(Keys::K));
    for (int i = 0; i < 3; ++i)
        AX_SUBMIT_EVENT(TestWindowResizeEvent(0, 0));

    f.process(stack);

    REQUIRE(received.size() == 8);
    for (int i = 0; i < 5; ++i)
        CHECK(received[i] == EventType::KeyPressed);
    for (int i = 5; i < 8; ++i)
        CHECK(received[i] == EventType::WindowResize);
}

TEST_CASE("Empty stack processes without crash") {
    EHFixture f;
    LayerStack stack;

    TestKeyPressedEvent e(Keys::A);
    AX_SUBMIT_EVENT(e);
    CHECK_NOTHROW(f.process(stack));
}

TEST_CASE("No events dispatched does not crash") {
    EHFixture f;
    LayerStack stack;
    TestLayer* layer = new TestLayer();
    stack.PushLayer(layer);

    CHECK_NOTHROW(f.process(stack));
    CHECK(layer->eventCount == 0);
}

// ─── Singleton lifecycle ──────────────────────────────────────────────────────

TEST_CASE("EventHandler singleton lifecycle") {
    Log::Init();
    EventHandler::Init();
    EventHandler& first = EventHandler::GetInstance();
    EventHandler::Init(); // second call — must warn and keep same instance
    EventHandler& second = EventHandler::GetInstance();
    CHECK(&first == &second);
    EventHandler::ShutDown();

    EventHandler::Init();
    CHECK_NOTHROW(EventHandler::GetInstance());
    EventHandler::ShutDown();
}

// ─── Overlay ordering ─────────────────────────────────────────────────────────

TEST_CASE("Overlays receive events before layers") {
    EHFixture f;
    LayerStack stack;

    std::vector<std::string> order;
    TestLayer* layerA = new TestLayer("LayerA", [&](Event&) { order.push_back("LayerA"); });
    TestLayer* layerB = new TestLayer("LayerB", [&](Event&) { order.push_back("LayerB"); });
    TestLayer* overlay = new TestLayer("Overlay", [&](Event&) { order.push_back("Overlay"); });

    stack.PushLayer(layerA);
    stack.PushLayer(layerB);
    stack.PushOverlay(overlay);

    TestKeyPressedEvent e(Keys::K);
    AX_SUBMIT_EVENT(e);
    f.process(stack);

    REQUIRE(order.size() == 3);
    CHECK(order[0] == "Overlay");
    CHECK(order[1] == "LayerB");
    CHECK(order[2] == "LayerA");
}

// ─── Thread safety ────────────────────────────────────────────────────────────

TEST_CASE("Concurrent DispatchEvent is thread-safe") {
    EHFixture f;
    LayerStack stack;

    std::atomic<int> eventCount{0};
    TestLayer* layer = new TestLayer("L1", [&](Event&) { eventCount.fetch_add(1, std::memory_order_relaxed); });
    stack.PushLayer(layer);

    constexpr int kThreads = 8;
    constexpr int kPerThread = 100;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kPerThread; ++i)
                AX_SUBMIT_EVENT(TestKeyPressedEvent(Keys::K));
        });
    }
    for (auto& th : threads)
        th.join();

    f.process(stack);

    CHECK(eventCount.load() == kThreads * kPerThread);
}
