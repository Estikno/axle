#include <doctest.h>

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

struct PlayerData {
	short id;
	float health;
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

void TestFunction_AllInput_With_Data(Event* event) {
	CHECK(event != nullptr);
	CHECK(event->GetEventType() == Axle::EventType::KeyPressed);
	CHECK(event->GetEventGategory() == Axle::EventCategory::Input);

	CHECK(event->GetContext().u16_values[0] == 12);
}

void TestFunction_Complex_CustomData(Event* event) {
	CHECK(event != nullptr);
	CHECK(event->GetEventType() == Axle::EventType::KeyPressed);
	CHECK(event->GetEventGategory() == Axle::EventCategory::Input);

	CHECK(event->GetContext().custom_data.size == sizeof(PlayerData));

	PlayerData* player = static_cast<PlayerData*>(event->GetContext().custom_data.data);

	CHECK(player->id == 12);
	CHECK(player->health == doctest::Approx(100.0f));

	delete player;
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

	SUBCASE("Event containing custom data") {
		Subscription sub_1 = instance.Subscribe(TestFunction_AllInput_With_Data, EventType::None, EventCategory::Input);

		Event* event_1 = new Event(EventType::KeyPressed, EventCategory::Input);
		event_1->GetContext().u16_values[0] = 12;

		CHECK_NOTHROW(AX_ADD_EVENT(event_1));
	}

	SUBCASE("Event containing complex custom data") {
		Subscription sub_1 = instance.Subscribe(TestFunction_Complex_CustomData, EventType::None, EventCategory::Input);

		PlayerData* pdata = new PlayerData{ 12, 100.0f };

		Event* event_1 = new Event(EventType::KeyPressed, EventCategory::Input);
		event_1->GetContext().custom_data.size = sizeof(PlayerData);
		event_1->GetContext().custom_data.data = pdata;

		CHECK_NOTHROW(AX_ADD_EVENT(event_1));
	}
}
