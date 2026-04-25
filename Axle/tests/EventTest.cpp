#include <doctest.h>

#include "Core/Logger/Log.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"

using namespace Axle;

class newEvent : public Event {
public:
    int p_data = 23;

    newEvent(EventType type, EventCategory category)
        : Event(type, category) {
        p_data = 1;
    }
};

struct PlayerData {
    short id;
    float health;
};

void TestFunction_Render(Event& event) {
    CHECK(event.GetEventType() == Axle::EventType::AppRender);
}

void TestFunction_Tick(Event& event) {
    CHECK(event.GetEventType() == Axle::EventType::AppTick);
}

void TestFunction_Input(Event& event) {
    CHECK(event.GetEventType() == Axle::EventType::AppUpdate);
}

void TestFunction_AllInput(Event& event) {
    CHECK(event.GetEventType() == Axle::EventType::KeyPressed);
    CHECK(event.GetEventCategory() == Axle::EventCategory::Input);
}

void TestFunction_AllInput_With_Data(Event& event) {
    CHECK(event.GetEventType() == Axle::EventType::KeyPressed);
    CHECK(event.GetEventCategory() == Axle::EventCategory::Input);
    CHECK_FALSE(event.GetContext().custom_data.has_value());

    CHECK(std::get<std::array<u16, 8>>(event.GetContext().raw_data).at(0) == 12);
}

void TestFunction_Complex_CustomData(Event& event) {
    CHECK(event.GetEventType() == Axle::EventType::KeyPressed);
    CHECK(event.GetEventCategory() == Axle::EventCategory::Input);
    CHECK(event.GetContext().custom_data.has_value());

    PlayerData* player = std::any_cast<PlayerData*>(event.GetContext().custom_data.value());

    CHECK(player->id == 12);
    CHECK(player->health == doctest::Approx(100.0f));

    delete player;
}

TEST_CASE("EventHandler") {
    Log::Init();
    EventHandler::Init();

    EventHandler& instance = EventHandler::GetInstance();

    SUBCASE("EventHandler AddEvent") {
        size_t id = instance.Subscribe(TestFunction_Render, EventType::AppRender, EventCategory::Render);
        size_t id2 = instance.Subscribe(TestFunction_Tick, EventType::AppTick, EventCategory::Render);

        Event event_1(EventType::AppRender, EventCategory::Render);
        Event event_2(EventType::AppTick, EventCategory::Render);

        CHECK_NOTHROW(AX_ADD_EVENT(event_1));
        CHECK_NOTHROW(AX_ADD_EVENT(event_2));

        instance.ProcessEvents();

        instance.Unsubscribe(id);
        instance.Unsubscribe(id2);
    }

    SUBCASE("EventHandler with inherited events") {
        size_t id = instance.Subscribe(TestFunction_Input, EventType::AppUpdate, EventCategory::Window);

        newEvent event_1(EventType::AppUpdate, EventCategory::Input);

        CHECK_NOTHROW(AX_ADD_EVENT(event_1));

        instance.ProcessEvents();

        instance.Unsubscribe(id);
    }

    SUBCASE("EventHandler receiving events of a whole group") {
        size_t id = instance.Subscribe(TestFunction_AllInput, EventType::None, EventCategory::Input);

        Event event_1(EventType::KeyPressed, EventCategory::Input);

        CHECK_NOTHROW(AX_ADD_EVENT(event_1));

        instance.ProcessEvents();

        instance.Unsubscribe(id);
    }

    SUBCASE("Event containing custom data") {
        size_t id = instance.Subscribe(TestFunction_AllInput_With_Data, EventType::None, EventCategory::Input);

        Event event_1(EventType::KeyPressed, EventCategory::Input);
        event_1.GetContext().raw_data = std::array<u16, 8>{12};

        CHECK_NOTHROW(AX_ADD_EVENT(event_1));

        instance.ProcessEvents();

        instance.Unsubscribe(id);
    }

    SUBCASE("Event containing complex custom data") {
        size_t id = instance.Subscribe(TestFunction_Complex_CustomData, EventType::None, EventCategory::Input);

        PlayerData* pdata = new PlayerData{12, 100.0f};

        Event event_1(EventType::KeyPressed, EventCategory::Input);
        event_1.GetContext().custom_data = pdata;

        CHECK_NOTHROW(AX_ADD_EVENT(event_1));

        instance.ProcessEvents();

        instance.Unsubscribe(id);
    }
}

// ─── Additional edge cases for EventHandler ───────────────────────────────

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static int g_callCount = 0; // reset before each subcase that uses it

// ---------------------------------------------------------------------------
// 1. Singleton lifecycle
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler singleton lifecycle") {
    // Double-Init should warn but not crash or reset the instance
    EventHandler::Init();
    EventHandler& first = EventHandler::GetInstance();
    EventHandler::Init(); // second call — should log warning, keep same instance
    EventHandler& second = EventHandler::GetInstance();
    CHECK(&first == &second);

    EventHandler::ShutDown();

    // Re-initializing after shutdown should work cleanly
    EventHandler::Init();
    CHECK_NOTHROW(EventHandler::GetInstance());
    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 2. No handlers registered — ProcessEvents must not crash
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler no subscribers") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    Event e(EventType::AppRender, EventCategory::Render);
    AX_ADD_EVENT(e);
    CHECK_NOTHROW(instance.ProcessEvents());

    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 3. Unsubscribe before processing — handler must NOT be called
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler unsubscribe before ProcessEvents") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    g_callCount = 0;
    size_t id = instance.Subscribe([](Event&) { g_callCount++; }, EventType::AppRender, EventCategory::Render);

    Event e(EventType::AppRender, EventCategory::Render);
    AX_ADD_EVENT(e);

    instance.Unsubscribe(id); // remove before processing
    instance.ProcessEvents();

    CHECK(g_callCount == 0);

    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 4. Unsubscribe an invalid/already-removed id — must not crash
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler double unsubscribe") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    size_t id = instance.Subscribe([](Event&) {}, EventType::AppTick, EventCategory::Render);
    instance.Unsubscribe(id);
    CHECK_NOTHROW(instance.Unsubscribe(id));   // second removal
    CHECK_NOTHROW(instance.Unsubscribe(9999)); // completely bogus id

    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 5. Multiple subscribers on the same type all get called
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler multiple subscribers same type") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    g_callCount = 0;
    auto counter = [](Event&) { g_callCount++; };

    size_t id1 = instance.Subscribe(counter, EventType::AppRender, EventCategory::Render);
    size_t id2 = instance.Subscribe(counter, EventType::AppRender, EventCategory::Render);
    size_t id3 = instance.Subscribe(counter, EventType::AppRender, EventCategory::Render);

    Event e(EventType::AppRender, EventCategory::Render);
    AX_ADD_EVENT(e);
    instance.ProcessEvents();

    CHECK(g_callCount == 3);

    instance.Unsubscribe(id1);
    instance.Unsubscribe(id2);
    instance.Unsubscribe(id3);

    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 6. Category-wildcard subscriber does NOT receive events from other categories
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler category isolation") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    g_callCount = 0;
    // Subscribed to Input category, EventType::None = any type within that category
    size_t id = instance.Subscribe([](Event&) { g_callCount++; }, EventType::None, EventCategory::Input);

    Event render_event(EventType::AppRender, EventCategory::Render);
    Event input_event(EventType::KeyPressed, EventCategory::Input);
    AX_ADD_EVENT(render_event);
    AX_ADD_EVENT(input_event);
    instance.ProcessEvents();

    CHECK(g_callCount == 1); // only the Input event should have matched

    instance.Unsubscribe(id);
    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 7. Marked-as-handled events stop propagation
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler handled event stops propagation") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    g_callCount = 0;

    // FIX: Once priority is implemented in the event handler we should test it here

    // First handler consumes the event
    size_t id1 = instance.Subscribe(
        [](Event& e) {
            g_callCount++;
            e.Handle();
        },
        EventType::AppRender,
        EventCategory::Render);
    // Second handler should never run
    size_t id2 = instance.Subscribe(
        [](Event& e) {
            g_callCount++;
            e.Handle();
        },
        EventType::AppRender,
        EventCategory::Render);

    Event e(EventType::AppRender, EventCategory::Render);
    AX_ADD_EVENT(e);
    instance.ProcessEvents();

    // Exactly one handler should have fired
    CHECK(g_callCount == 1);

    instance.Unsubscribe(id1);
    instance.Unsubscribe(id2);
    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 8. ProcessEvents drains the queue — calling it twice processes each event once
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler queue is drained after ProcessEvents") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    g_callCount = 0;
    size_t id = instance.Subscribe([](Event&) { g_callCount++; }, EventType::AppRender, EventCategory::Render);

    Event e(EventType::AppRender, EventCategory::Render);
    AX_ADD_EVENT(e);
    instance.ProcessEvents(); // processes the one event
    instance.ProcessEvents(); // queue is empty — nothing extra fires

    CHECK(g_callCount == 1);

    instance.Unsubscribe(id);
    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 9. Multiple events of different types — each goes to its own subscriber only
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler type routing") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    int renderCount = 0, tickCount = 0;

    size_t id1 = instance.Subscribe([&](Event&) { renderCount++; }, EventType::AppRender, EventCategory::Render);
    size_t id2 = instance.Subscribe([&](Event&) { tickCount++; }, EventType::AppTick, EventCategory::Render);

    for (int i = 0; i < 5; ++i)
        AX_ADD_EVENT(Event(EventType::AppRender, EventCategory::Render));
    for (int i = 0; i < 3; ++i)
        AX_ADD_EVENT(Event(EventType::AppTick, EventCategory::Render));

    instance.ProcessEvents();

    CHECK(renderCount == 5);
    CHECK(tickCount == 3);

    instance.Unsubscribe(id1);
    instance.Unsubscribe(id2);
    EventHandler::ShutDown();
}

// ---------------------------------------------------------------------------
// 10. Thread safety — concurrent AddEvent calls must not corrupt the queue
// ---------------------------------------------------------------------------

TEST_CASE("EventHandler concurrent AddEvent") {
    Log::Init();
    EventHandler::Init();
    EventHandler& instance = EventHandler::GetInstance();

    constexpr int kThreads = 8;
    constexpr int kPerThread = 100;

    g_callCount = 0;
    size_t id = instance.Subscribe([](Event&) { g_callCount++; }, // NOTE: g_callCount itself is not atomic here;
                                   EventType::AppRender,
                                   EventCategory::Render // if ProcessEvents is single-threaded this is fine
    );

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kPerThread; ++i)
                AX_ADD_EVENT(Event(EventType::AppRender, EventCategory::Render));
        });
    }
    for (auto& th : threads)
        th.join();

    instance.ProcessEvents();

    CHECK(g_callCount == kThreads * kPerThread);

    instance.Unsubscribe(id);
    EventHandler::ShutDown();
}
