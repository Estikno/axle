#include <doctest.h>

#include "Core/Logger/Log.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Layer/Layer.hpp"
#include "Core/Layer/LayerStack.hpp"

using namespace Axle;

// ─── Helpers ────────────────────────────────────────────────────────────────

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
        eventCount++;
        event.Handle();
    }
};

// Custom event subclass used across multiple test cases
class DerivedEvent : public Event {
public:
    int p_data = 0;

    DerivedEvent(EventType type, EventCategory category)
        : Event(type, category) {
        p_data = 1;
    }
};

struct PlayerData {
    short id;
    float health;
};

struct EHFixture {
    EHFixture() {
        Log::Init();
        EventHandler::Init();
    }
    ~EHFixture() {
        EventHandler::ShutDown();
    }
};

// ─── Core functionality ──────────────────────────────────────────────────────

TEST_CASE("Single layer receives dispatched event") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;
    TestLayer* layer = new TestLayer("L1", [](Event& e) {
        CHECK(e.GetEventType() == EventType::AppRender);
        CHECK(e.GetEventCategory() == EventCategory::Render);
    });
    stack.PushLayer(layer);

    Event e(EventType::AppRender, EventCategory::Render);
    CHECK_NOTHROW(AX_DISPATCH_EVENT(e));
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    CHECK(layer->eventCount == 1);
}

TEST_CASE("Multiple layers each receive the event") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;
    TestLayer* l1 = new TestLayer("L1");
    TestLayer* l2 = new TestLayer("L2");
    TestLayer* l3 = new TestLayer("L3");

    stack.PushLayer(l1);
    stack.PushLayer(l2);
    stack.PushLayer(l3);

    Event e(EventType::AppTick, EventCategory::Render);
    AX_DISPATCH_EVENT(e);
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    CHECK(l1->eventCount == 1);
    CHECK(l2->eventCount == 1);
    CHECK(l3->eventCount == 1);
}

TEST_CASE("Inherited event subclass is dispatched correctly") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;
    TestLayer* layer = new TestLayer("L1", [](Event& e) { CHECK(e.GetEventType() == EventType::AppUpdate); });
    stack.PushLayer(layer);

    DerivedEvent e(EventType::AppUpdate, EventCategory::Input);
    CHECK_NOTHROW(AX_DISPATCH_EVENT(e));
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    CHECK(layer->eventCount == 1);
}

TEST_CASE("Event with raw_data context is forwarded intact") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;
    TestLayer* layer = new TestLayer("L1", [](Event& e) {
        CHECK(e.GetEventType() == EventType::KeyPressed);
        CHECK(e.GetEventCategory() == EventCategory::Input);
        CHECK_FALSE(e.GetContext().custom_data.has_value());
        CHECK(std::get<std::array<u16, 8>>(e.GetContext().raw_data).at(0) == 12);
    });
    stack.PushLayer(layer);

    Event e(EventType::KeyPressed, EventCategory::Input);
    e.GetContext().raw_data = std::array<u16, 8>{12};
    CHECK_NOTHROW(AX_DISPATCH_EVENT(e));
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    CHECK(layer->eventCount == 1);
}

TEST_CASE("Event with complex custom_data context is forwarded intact") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;
    TestLayer* layer = new TestLayer("L1", [](Event& e) {
        CHECK(e.GetEventType() == EventType::KeyPressed);
        CHECK(e.GetEventCategory() == EventCategory::Input);
        REQUIRE(e.GetContext().custom_data.has_value());

        PlayerData* player = std::any_cast<PlayerData*>(e.GetContext().custom_data.value());
        CHECK(player->id == 12);
        CHECK(player->health == doctest::Approx(100.0f));
        delete player;
    });
    stack.PushLayer(layer);

    PlayerData* pdata = new PlayerData{12, 100.0f};
    Event e(EventType::KeyPressed, EventCategory::Input);
    e.GetContext().custom_data = pdata;
    CHECK_NOTHROW(AX_DISPATCH_EVENT(e));
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    CHECK(layer->eventCount == 1);
}

// ─── Propagation & handled-event semantics ──────────────────────────────────

TEST_CASE("EventHandler handled event stops propagation through layer stack") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;

    // Layers are iterated rbegin→rend, i.e. top of stack first.
    // ConsumingLayer is pushed as overlay so it sits on top and fires first.
    TestLayer* bottom = new TestLayer("Bottom");
    ConsumingLayer* top = new ConsumingLayer("Top");
    stack.PushLayer(bottom);
    stack.PushOverlay(top); // overlay → processed before bottom

    Event e(EventType::AppRender, EventCategory::Render);
    AX_DISPATCH_EVENT(e);
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    CHECK(top->eventCount == 1);    // consumed here
    CHECK(bottom->eventCount == 0); // never reached
}

TEST_CASE("EventHandler unhandled event reaches all layers") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;
    TestLayer* l1 = new TestLayer("L1");
    TestLayer* l2 = new TestLayer("L2");
    stack.PushLayer(l1);
    stack.PushLayer(l2);

    Event e(EventType::AppRender, EventCategory::Render);
    AX_DISPATCH_EVENT(e);
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    CHECK(l1->eventCount == 1);
    CHECK(l2->eventCount == 1);
}

// ─── Queue semantics ─────────────────────────────────────────────────────────

TEST_CASE("EventHandler queue is drained after ProcessEvents") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;
    TestLayer* layer = new TestLayer();
    stack.PushLayer(layer);

    Event e(EventType::AppRender, EventCategory::Render);
    AX_DISPATCH_EVENT(e);
    instance.ProcessEvents(stack.rbegin(), stack.rend()); // drains the queue
    instance.ProcessEvents(stack.rbegin(), stack.rend()); // nothing left

    CHECK(layer->eventCount == 1);
}

TEST_CASE("EventHandler processes multiple events in dispatch order") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;

    std::vector<EventType> received;
    TestLayer* layer = new TestLayer("L1", [&](Event& e) { received.push_back(e.GetEventType()); });
    stack.PushLayer(layer);

    for (int i = 0; i < 5; ++i)
        AX_DISPATCH_EVENT(Event(EventType::AppRender, EventCategory::Render));
    for (int i = 0; i < 3; ++i)
        AX_DISPATCH_EVENT(Event(EventType::AppTick, EventCategory::Render));

    instance.ProcessEvents(stack.rbegin(), stack.rend());

    REQUIRE(received.size() == 8);
    for (int i = 0; i < 5; ++i)
        CHECK(received[i] == EventType::AppRender);
    for (int i = 5; i < 8; ++i)
        CHECK(received[i] == EventType::AppTick);
}

TEST_CASE("EventHandler empty stack processes without crash") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack; // no layers pushed

    Event e(EventType::AppRender, EventCategory::Render);
    AX_DISPATCH_EVENT(e);
    CHECK_NOTHROW(instance.ProcessEvents(stack.rbegin(), stack.rend()));
}

TEST_CASE("EventHandler no events dispatched does not crash") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;
    TestLayer* layer = new TestLayer();
    stack.PushLayer(layer);

    CHECK_NOTHROW(instance.ProcessEvents(stack.rbegin(), stack.rend()));
    CHECK(layer->eventCount == 0);
}

// ─── Singleton lifecycle ─────────────────────────────────────────────────────

TEST_CASE("EventHandler singleton lifecycle") {
    // Double-Init must warn but keep the same instance without crashing
    EventHandler::Init();
    EventHandler& first = EventHandler::GetInstance();
    EventHandler::Init(); // second call — should log warning, keep same instance
    EventHandler& second = EventHandler::GetInstance();
    CHECK(&first == &second);

    EventHandler::ShutDown();

    // Re-initializing after shutdown must work cleanly
    EventHandler::Init();
    CHECK_NOTHROW(EventHandler::GetInstance());
    EventHandler::ShutDown();
}

// ─── Thread safety ────────────────────────────────────────────────────────────

TEST_CASE("EventHandler concurrent DispatchEvent is thread-safe") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

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
                AX_DISPATCH_EVENT(Event(EventType::AppRender, EventCategory::Render));
        });
    }
    for (auto& th : threads)
        th.join();

    // ProcessEvents is called from a single thread after all producers are done
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    CHECK(eventCount.load() == kThreads * kPerThread);
}

// ─── Overlay vs layer ordering ───────────────────────────────────────────────

TEST_CASE("EventHandler overlays receive events before layers") {
    EHFixture f;
    EventHandler& instance = EventHandler::GetInstance();

    LayerStack stack;

    std::vector<std::string> order;
    TestLayer* layerA = new TestLayer("LayerA", [&](Event&) { order.push_back("LayerA"); });
    TestLayer* layerB = new TestLayer("LayerB", [&](Event&) { order.push_back("LayerB"); });
    TestLayer* overlay = new TestLayer("Overlay", [&](Event&) { order.push_back("Overlay"); });

    stack.PushLayer(layerA);
    stack.PushLayer(layerB);
    stack.PushOverlay(overlay); // always sits on top

    Event e(EventType::AppRender, EventCategory::Render);
    AX_DISPATCH_EVENT(e);
    instance.ProcessEvents(stack.rbegin(), stack.rend());

    // rbegin→rend means overlay (top) fires first
    REQUIRE(order.size() == 3);
    CHECK(order[0] == "Overlay");
    // LayerB and LayerA follow in reverse push order
    CHECK(order[1] == "LayerB");
    CHECK(order[2] == "LayerA");
}
