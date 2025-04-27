#pragma once

#include "Application.hpp"
#include "Logger/Log.hpp"
#include "Events/EventHandler.hpp"

#ifdef AX_PLATFORM_WINDOWS

//delete
//---
#include "Events/Event.hpp"
#include "Core.hpp"
//---

// This function is created by the application and not the engine
extern Axle::Application *Axle::CreateApplication();

/*
* The main function is excecuted by the engine and not by the application.
*
* This gives the clients the only purpose of designing their app, instead
of focusing on making the main loop.
*/
int main(int argc, char **argv) {
    Axle::Log::Init();
    Axle::EventHandler::Init();

    // Test purposes (delete)
    // ---------------------
    auto object = Axle::EventHandler::GetInstance()->Subscribe(Axle::test_method, Axle::EventType::WindowClose);

    Axle::Event* newEvent = new Axle::Event(Axle::EventType::WindowClose, Axle::EventCategory::Window);
    AX_ADD_EVENT(newEvent);
    //---------------------

    // Main application loop
    Axle::Application* app = Axle::CreateApplication();
    app->Run();
    delete app;
}

#endif // AX_PLATFORM_WINDOWS
