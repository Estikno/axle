#pragma once

#ifdef AX_PLATFORM_WINDOWS

// This function is created by the application and not the engine
extern Axle::Application* Axle::CreateApplication();

/*
* The main function is excecuted by the engine and not by the application.
*
* This gives the clients the only purpose of designing their app, instead 
of focusing on making the main loop.
*/
int main(int argc, char** argv) {
	Axle::Log::Init();
	AX_CORE_WARN("Using a macro!");
	AX_CRITICAL("Client critical error");

	auto app = Axle::CreateApplication();
	app->Run();
	delete app;
}

#endif // AX_PLATFORM_WINDOWS
