#include "axpch.hpp"

#include "Application.hpp"
#include "Logger/Log.hpp"
#include "Input/Input.hpp"
#include "Input/InputCallbacks.hpp"
#include "Math/Math_Types.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
	void ErrorCallback(int error, const char* description) {
		AX_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Application::Application() {
		AX_CORE_TRACE("Starting the engine...");

		if (!glfwInit()) {
			AX_CORE_ERROR("Failed to initialize GLFW");

			m_HasErrorInit = true;

			return;
		}

		glfwSetErrorCallback(ErrorCallback);

		m_window = glfwCreateWindow(1280, 720, "Axle Engine", nullptr, nullptr);

		if (!m_window) {
			AX_CORE_ERROR("Failed to create GLFW window");
			glfwTerminate();
			m_window = nullptr;

			m_HasErrorInit = true;

			return;
		}

		glfwSetKeyCallback(m_window, KeyCallback);
		glfwSetCursorPosCallback(m_window, CursorPositionCallback);
		glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
		glfwSetScrollCallback(m_window, ScrollCallback);

		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(1);
	}

	Application::~Application() {
		AX_CORE_TRACE("Stopping the engine...");

		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Application::Run() {
		if (m_HasErrorInit) return;

		while (!glfwWindowShouldClose(m_window)) {

			// NOTE: Input state updating should be performed at the end of each frame.
			// The input is recorded in between the frame but the update happens at the end.
			Input::Update();
			glfwPollEvents();
		}
	}
}
