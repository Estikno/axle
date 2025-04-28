#include "Application.hpp"
#include "Logger/Log.hpp"

namespace Axle {
    Application::Application(){
        AX_CORE_TRACE("Starting the engine...");
	}

	Application::~Application() {
        AX_CORE_TRACE("Stopping the engine...");
	}

	void Application::Run() {
		while (true) {

		}
	}
}
