#pragma once

#include "axpch.hpp"

#include "Application.hpp"
#include "Events/EventHandler.hpp"
#include "Logger/Log.hpp"

#ifdef AX_PLATFORM_WINDOWS

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

    // Main application loop
    Axle::Application *app = Axle::CreateApplication();
    app->Run();
    delete app;
}

#endif // AX_PLATFORM_WINDOWS

#ifdef AX_PLATFORM_LINUX

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

    // Main application loop
    Axle::Application *app = Axle::CreateApplication();
    app->Run();
    delete app;
}

#endif 
