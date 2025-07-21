#include "axpch.hpp"

#include "Application.hpp"
#include "Logger/Log.hpp"
#include "Input/Input.hpp"

namespace Axle {
    Application::Application(){
        AX_CORE_TRACE("Starting the engine...");
	}

	Application::~Application() {
        AX_CORE_TRACE("Stopping the engine...");
	}

	void Application::Run() {
		while (true) {
			// NOTE: Input state updating should be performed at the end of each frame.
			// The input is recorded in between the frame but the update happens at the end.
			Input::Update();
		}
	}
}
