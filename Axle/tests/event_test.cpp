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

void TestFunction_Render(Event* event) {
	CHECK(event != nullptr);
	CHECK(event->GetEventType() == Axle::EventType::AppRender);
}

void TestFunction_Tick(Event* event) {
	CHECK(event != nullptr);
	CHECK(event->GetEventType() == Axle::EventType::AppTick);
}

void TestFunction_Input(Event* event) {
	CHECK(event != nullptr);
	CHECK(event->GetEventType() == Axle::EventType::AppUpdate);
}

void TestFunction_AllInput(Event* event) {
	CHECK(event != nullptr);
	CHECK(event->GetEventType() == Axle::EventType::KeyPressed);
	CHECK(event->GetEventGategory() == Axle::EventCategory::Input);
}

TEST_CASE("EventHandler") {
	Log::Init();
	EventHandler::Init();

	EventHandler& instance = EventHandler::GetInstance();

	SUBCASE("EventHandler AddEvent") {
		Subscription sub_1 = instance.Subscribe(TestFunction_Render, EventType::AppRender, EventCategory::Render);
		Subscription sub_2 = instance.Subscribe(TestFunction_Tick, EventType::AppTick, EventCategory::Window);

		Event* event_1 = new Event(EventType::AppRender, EventCategory::Render);
		Event* event_2 = new Event(EventType::AppTick, EventCategory::Render);

		CHECK_NOTHROW(AX_ADD_EVENT(event_1));
		CHECK_NOTHROW(AX_ADD_EVENT(event_2));
	}

	SUBCASE("EventHandler with inherited events") {
		Subscription sub_1 = instance.Subscribe(TestFunction_Input, EventType::AppUpdate, EventCategory::Window);

		newEvent* event_1 = new newEvent(EventType::AppUpdate, EventCategory::Input);

		CHECK_NOTHROW(AX_ADD_EVENT(event_1));
	}

	SUBCASE("EventHandler receiving events of a whole group") {
		Subscription sub_1 = instance.Subscribe(TestFunction_AllInput, EventType::None, EventCategory::Input);

		Event* event_1 = new Event(EventType::KeyPressed, EventCategory::Input);

		CHECK_NOTHROW(AX_ADD_EVENT(event_1));
	}
}
