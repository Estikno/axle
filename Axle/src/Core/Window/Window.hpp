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

    class AXLE_API Window {
    public:
        using EventCallbackFn = std::function<void(Event*)>;

        Window(const WindowProps& props);
        ~Window();

        void OnUpdate();

        inline u32 GetWidth() const {
            return m_Data.Width;
        }
        inline u32 GetHeight() const {
            return m_Data.Height;
        }

        inline void SetEventCallback(const EventCallbackFn& callback) {
            m_Data.EventCallback = callback;
        }
        void SetVSync(bool enabled);
        inline bool IsVSync() const {
            return m_Data.VSync;
        }

        static Window* Create(const WindowProps& props = WindowProps());

    private:
        GLFWwindow* m_Window;

        struct WindowData {
            std::string Title;
            u32 Width, Height;
            bool VSync;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };
} // namespace Axle
