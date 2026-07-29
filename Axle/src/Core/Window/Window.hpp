#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"

struct GLFWwindow; // forward declaration — avoids pulling in glfw3.h here

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
        u32 Width = 0, Height = 0;
        u32 FramebufferWidth = 0, FramebufferHeight = 0;
        bool VSync = true;
    };

    class AXLE_API Window {
    public:
        Window(const WindowProps& props);
        ~Window();

        /**
         * Method to be called every frame to update the window.
         * */
        void OnUpdate();
        void PollEvents();

        inline u32 GetWidth() const noexcept {
            return m_Data.Width;
        }
        inline u32 GetHeight() const noexcept {
            return m_Data.Height;
        }
        inline bool IsVSync() const noexcept {
            return m_Data.VSync;
        }

        /**
         * Enable or not vertical synchronization.
         *
         * @param enabled True to enable VSync, false to disable it.
         * */
        void SetVSync(bool enabled);

        inline const WindowData& GetWindowData() const {
            return m_Data;
        }

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
