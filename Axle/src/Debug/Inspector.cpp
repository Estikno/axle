#include "axpch.hpp"

#include "Inspector.hpp"

#include "glm/gtc/type_ptr.hpp"
#include <imgui.h>
#include "Core/Application.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle::Debug {
    std::unique_ptr<Inspector> Inspector::s_Instance = nullptr;

    void Inspector::Init() {
        s_Instance = std::make_unique<Inspector>();
    }

    void Inspector::Shutdown() {
        s_Instance.reset();
    }

    void Inspector::Draw(const char* title, bool* open) {
        // ImGui::SetNextWindowSize(ImVec2(300, 400));
        if (!ImGui::Begin(title, open)) {
            ImGui::End();
            return;
        }

        // Camera header
        if (ImGui::CollapsingHeader("Camera")) {
            const char* CameraTypesNames[2] = {"Debug", "Move To"};
            static i32 previousSelectedIndex = 0;
            static i32 selectedIndex = 0;

            if (ImGui::BeginCombo("Camera Type", CameraTypesNames[selectedIndex])) {
                for (u8 i = 0; i < 2; ++i) {
                    const bool isSelected = (selectedIndex == i);
                    if (ImGui::Selectable(CameraTypesNames[i], isSelected))
                        selectedIndex = i;

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (previousSelectedIndex != selectedIndex) {
                previousSelectedIndex = selectedIndex;

                ICameraPositioner* old = nullptr;

                if (selectedIndex == 0)
                    old = Application::GetInstance().GetCamera().ExchangePositioner(new CameraPositionerDebug());
                else if (selectedIndex == 1)
                    old = Application::GetInstance().GetCamera().ExchangePositioner(new CameraPositionerMoveTo());

                if (old != nullptr)
                    delete old;
            }

            if (selectedIndex == 1) {
                static glm::vec3 pos = glm::vec3(0.0f);
                if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.005f)) {
                    // Set desired position to move to camera
                }
                static glm::vec3 orientation = glm::vec3(0.0f);
                if (ImGui::DragFloat3("Pitch/Yaw/Roll",
                                      glm::value_ptr(orientation),
                                      0.005f,
                                      0.0f,
                                      359.999f,
                                      "%.3f",
                                      ImGuiSliderFlags_WrapAround)) {
                    // Set desired orientation to move to camera
                }
            }
        }

        // Custom headers
        for (auto& f : s_Instance->m_Headers) {
            f();
        }

        ImGui::End();
    }
} // namespace Axle::Debug
