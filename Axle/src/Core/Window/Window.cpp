#include "axpch.hpp"

#include "Window.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "Callbacks/InputCallbacks.hpp"
#include "Callbacks/WindowCallbacks.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Axle {
    static bool s_IsGlfwInitialized = false;
    static u8 active_windows = 0;

    static void ErrorCallback(int error, const char* description) {
        AX_CORE_ERROR(LogChannel::Window, "GLFW Error ({0}): {1}", error, description);
    }

    static void APIENTRY glDebugOutput(GLenum source,
                                       GLenum type,
                                       u32 id,
                                       GLenum severity,
                                       GLsizei length,
                                       const char* message,
                                       const void* userParam);

    Window* Window::Create(const WindowProps& props) {
        return new Window(props);
    }

    Window::Window(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        AX_CORE_INFO(
            LogChannel::Window, "Creating window with title: {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (!s_IsGlfwInitialized) {
            i32 success = glfwInit();
            AX_ASSERT(success, LogChannel::Window, "Could not initialize GLFW!");

            // Define openGL version (4.6 Core)
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef AX_DEBUG
            // Enable debug output
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif // AX_DEBUG

            glfwSetErrorCallback(ErrorCallback);
            s_IsGlfwInitialized = true;
        }

        // Creating the actual window with GLFW
        m_Window = glfwCreateWindow((int) props.Width, (int) props.Height, props.Title.c_str(), nullptr, nullptr);
        AX_ASSERT(m_Window, LogChannel::Window, "Failed to create GLFW window!");

        ++active_windows;

        glfwMakeContextCurrent(m_Window);

        // Initialize Glad
        int version = gladLoaderLoadGL();
        AX_ASSERT(version != 0, LogChannel::Window, "Failed to initialize Glad!");
        glViewport(0, 0, props.Width, props.Height);

        AX_CORE_INFO(
            LogChannel::Window, "Loaded OpenGL {0}.{1}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

        // Set a way to retrieve the window data from the GLFW window easily
        glfwSetWindowUserPointer(m_Window, &m_Data);
        // By default enable VSync
        SetVSync(true);

        // Window callbacks
        glfwSetWindowCloseCallback(m_Window, WindowCloseCallback);
        glfwSetWindowSizeCallback(m_Window, WindowSizeCallback);
        glfwSetFramebufferSizeCallback(m_Window, FrameBufferSizeCallback);

        // Input callbacks
        glfwSetKeyCallback(m_Window, KeyCallback);
        glfwSetCursorPosCallback(m_Window, CursorPositionCallback);
        glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
        glfwSetScrollCallback(m_Window, ScrollCallback);

#ifdef AX_DEBUG
        // Check if debug output was correctly initialized
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            AX_CORE_TRACE(LogChannel::Window, "Enabled OpenGL debug ouput");
        } else {
            AX_CORE_TRACE(LogChannel::Window, "Couldn't enable OpenGL debug ouput");
        }
#endif // AX_DEBUG

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    Window::~Window() {
        gladLoaderUnloadGL();
        glfwDestroyWindow(m_Window);

        // If it's the last window terminate GLFW
        --active_windows;
        if (active_windows == 0) {
            glfwTerminate();
            s_IsGlfwInitialized = false;
            AX_CORE_INFO(LogChannel::Window, "GLFW terminated successfully.");
        }
    }

    void Window::OnUpdate() {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

    void Window::SetVSync(bool enabled) {
        glfwSwapInterval(enabled ? 1 : 0);
        m_Data.VSync = enabled;
    }

    static void APIENTRY glDebugOutput(GLenum source,
                                       GLenum type,
                                       u32 id,
                                       GLenum severity,
                                       GLsizei length,
                                       const char* message,
                                       const void* userParam) {
        // ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
            return;

        const char* sourceText;
        const char* typeText;

        switch (source) {
            case GL_DEBUG_SOURCE_API:
                sourceText = "Source: API";
                break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                sourceText = "Source: Window System";
                break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                sourceText = "Source: Shader Compiler";
                break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                sourceText = "Source: Third Party";
                break;
            case GL_DEBUG_SOURCE_APPLICATION:
                sourceText = "Source: Application";
                break;
            case GL_DEBUG_SOURCE_OTHER:
                sourceText = "Source: Other";
                break;
        }

        switch (type) {
            case GL_DEBUG_TYPE_ERROR:
                typeText = "Type: Error";
                break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                typeText = "Type: Deprecated Behaviour";
                break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                typeText = "Type: Undefined Behaviour";
                break;
            case GL_DEBUG_TYPE_PORTABILITY:
                typeText = "Type: Portability";
                break;
            case GL_DEBUG_TYPE_PERFORMANCE:
                typeText = "Type: Performance";
                break;
            case GL_DEBUG_TYPE_MARKER:
                typeText = "Type: Marker";
                break;
            case GL_DEBUG_TYPE_PUSH_GROUP:
                typeText = "Type: Push Group";
                break;
            case GL_DEBUG_TYPE_POP_GROUP:
                typeText = "Type: Pop Group";
                break;
            case GL_DEBUG_TYPE_OTHER:
                typeText = "Type: Other";
                break;
        }

        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
                AX_CORE_ERROR(LogChannel::Renderer,
                              "[OpenGL Message] {0}, {1}, Message ({2}): {3}",
                              sourceText,
                              typeText,
                              id,
                              message);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                AX_CORE_WARN(LogChannel::Renderer,
                             "[OpenGL Message] {0}, {1}, Message ({2}): {3}",
                             sourceText,
                             typeText,
                             id,
                             message);
                break;
            case GL_DEBUG_SEVERITY_LOW:
                AX_CORE_WARN(LogChannel::Renderer,
                             "[OpenGL Message] {0}, {1}, Message ({2}): {3}",
                             sourceText,
                             typeText,
                             id,
                             message);
                break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                AX_CORE_INFO(LogChannel::Renderer,
                             "[OpenGL Message] {0}, {1}, Message ({2}): {3}",
                             sourceText,
                             typeText,
                             id,
                             message);
                break;
        }
    }
} // namespace Axle
