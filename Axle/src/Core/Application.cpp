#include "axpch.hpp"

#include "Application.hpp"
#include "Logger/Log.hpp"
#include "Input/Input.hpp"
#include "Math/Math_Types.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
	static void ErrorCallback(int error, const char* description) {
		AX_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		// Convert the GLFW key to our own key enum
		Keys key_enum = ConvertGLFWKeys(key);

		if (key_enum == Keys::Unknown) {
			AX_CORE_WARN("Unknown key pressed: {0}", key);
			return;
		}

		if (action == GLFW_KEY_UNKNOWN) {
			AX_CORE_WARN("Unknown action for key: {0}", key);
			return;
		}

		// Sends the status to the input system
		Input::SetKey(key_enum, !(action == GLFW_RELEASE));
	}

	static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
		Vector2 position(static_cast<float>(xpos), static_cast<float>(ypos));
		Input::SetMousePosition(position);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		// Convert the GLFW button to our own mouse buttons enum
		MouseButtons button_enum = ConvertGLFWMouseButtons(button);

		if (button_enum == MouseButtons::Unknown) {
			AX_CORE_WARN("Unknown mouse button pressed: {0}", button);
			return;
		}

		Input::SetMouseButton(button_enum, action == GLFW_PRESS);
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
