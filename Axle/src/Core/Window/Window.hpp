#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"

#include "Core/Events/Event.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    struct WindowProps {
        std::string Title;
        u32 Width;
        u32 Height;

        WindowProps(const std::string& title = "Axle Engine", u32 width = 1280, u32 height = 720)
            : Title(title),
              Width(width),
              Height(height) {}
    };

    struct WindowData {
        std::string Title;
        u32 Width, Height;
        u32 FramebufferWidth, FramebufferHeight;
        bool VSync;
    };

    class AXLE_API Window {
    public:
        using EventCallbackFn = std::function<void(Event*)>;

        Window(const WindowProps& props);
        ~Window();

        /**
         * Method to be called every frame to update the window.
         * */
        void OnUpdate();

        inline u32 GetWidth() const {
            return m_Data.Width;
        }
        inline u32 GetHeight() const {
            return m_Data.Height;
        }
        inline bool IsVSync() const {
            return m_Data.VSync;
        }

        /**
         * Enable or not vertical synchronization.
         *
         * @param enabled True to enable VSync, false to disable it.
         * */
        void SetVSync(bool enabled);

        inline GLFWwindow* GetNativeWindow() {
            return m_Window;
        }

        /**
         * Creates a new window given the properties.
         *
         * @param props The properties of the window to be created.
         *
         * @returns A pointer to the created window.
         * */
        static Window* Create(const WindowProps& props = WindowProps());

    private:
        /// The underlying glfw window
        GLFWwindow* m_Window;
        WindowData m_Data;
    };
} // namespace Axle
