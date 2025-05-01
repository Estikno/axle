#include "doctest.h"

#include "Core/Logger/Log.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"

using namespace Axle;

class newEvent : public Event {
public:
	int p_data = 23;

	newEvent(EventType type, EventCategory category) : Event(type, category) {
		p_data = 1;
	}
};

void TestFunction_Render(Axle::EventType type, Axle::Event* event) {
	CHECK(event != nullptr);
	CHECK(type == Axle::EventType::AppRender);
	CHECK(type == event->GetEventType());
}

void TestFunction_Tick(Axle::EventType type, Axle::Event* event) {
	CHECK(event != nullptr);
	CHECK(type == Axle::EventType::AppTick);
	CHECK(type == event->GetEventType());
}

void TestFunction_Input(Axle::EventType type, Axle::Event* event) {
	CHECK(event != nullptr);
	CHECK(type == Axle::EventType::AppUpdate);
	CHECK(type == event->GetEventType());
}

TEST_CASE("EventHandler") {
	Log::Init();
	EventHandler::Init();
	auto instance = EventHandler::GetInstance();

	CHECK(instance != nullptr);

	SUBCASE("Eventhandler AddEvent") {
		Subscription sub_1 = instance->Subscribe(TestFunction_Render, EventType::AppRender);
		Subscription sub_2 = instance->Subscribe(TestFunction_Tick, EventType::AppTick);

		Event* event_1 = new Event(EventType::AppRender, EventCategory::Render);
		Event* event_2 = new Event(EventType::AppTick, EventCategory::Render);

		CHECK_NOTHROW(AX_ADD_EVENT(event_1));
		CHECK_NOTHROW(AX_ADD_EVENT(event_2));
	}

	SUBCASE("Eventhandler with inherited events") {
		Subscription sub_1 = instance->Subscribe(TestFunction_Input, EventType::AppUpdate);

		newEvent* event_1 = new newEvent(EventType::AppUpdate, EventCategory::Input);

		CHECK_NOTHROW(AX_ADD_EVENT(event_1));
	}
}
