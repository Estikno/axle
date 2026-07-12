#include "axpch.hpp"

#include "Inspector.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "Core/Application.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Core/Logger/Log.hpp"

#include <imgui.h>

namespace Axle::Debug {
    static void HelpMarker(const char* desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

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

        // Headers
        s_Instance->CameraHeader();
        s_Instance->LogHeader();

        // Custom headers
        for (auto& f : s_Instance->m_Headers) {
            f();
        }

        ImGui::End();
    }

    void Inspector::CameraHeader() {
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
    }


    void Inspector::LogHeader() {
        if (ImGui::CollapsingHeader("Logger")) {
            // Select verbosity
            static i32 currentVerbosity = static_cast<i32>(Log::GetInstance().GetCurrentVerbosity());
            static i32 selectedIndex = currentVerbosity;
            static u8 size = static_cast<u8>(LogVerbosity::MaxVerbosities);

            if (ImGui::BeginCombo("Verbosity", std::string(VERBOSITY_NAMES[selectedIndex]).c_str())) {
                for (u8 i = 0; i < size; ++i) {
                    const bool isSelected = selectedIndex == i;
                    if (ImGui::Selectable(std::string(VERBOSITY_NAMES[i]).c_str(), isSelected))
                        selectedIndex = i;

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (currentVerbosity != selectedIndex) {
                currentVerbosity = selectedIndex;
                if (selectedIndex < 3)
                    AX_CORE_INFO(LogChannel::Debug, "Set log verbosity to: {0}", VERBOSITY_NAMES[selectedIndex]);
                Log::GetInstance().SetVerbosity(VerbosityFromStr(std::string(VERBOSITY_NAMES[selectedIndex])));
                if (selectedIndex >= 3)
                    AX_CORE_INFO(LogChannel::Debug, "Set log verbosity to: {0}", VERBOSITY_NAMES[selectedIndex]);
            }

            // Enable/Disable channels
            if (ImGui::TreeNode("Enable/Disable channels")) {
                std::array<bool, static_cast<u8>(LogChannel::MaxChannels)> currentEnables;
                std::array<bool, static_cast<u8>(LogChannel::MaxChannels)> selectEnables;

                for (u8 i = 0; i < currentEnables.size(); ++i) {
                    currentEnables[i] = Log::GetInstance().IsChannelEnabled(static_cast<LogChannel>(i));
                    selectEnables[i] = Log::GetInstance().IsChannelEnabled(static_cast<LogChannel>(i));

                    ImGui::Checkbox(std::string(CHANNEL_NAMES[i]).c_str(), &selectEnables[i]);
                }

                for (u8 i = 0; i < currentEnables.size(); ++i) {
                    if (selectEnables[i] != currentEnables[i]) {
                        if (selectEnables[i]) {
                            Log::GetInstance().EnableChannel(static_cast<LogChannel>(i));
                            AX_CORE_INFO(LogChannel::Debug, "Channel {0} got enabled", CHANNEL_NAMES[i]);
                        } else {
                            AX_CORE_INFO(LogChannel::Debug, "Channel {0} got disabled.", CHANNEL_NAMES[i]);
                            Log::GetInstance().DisableChannel(static_cast<LogChannel>(i));
                        }
                    }
                }

                ImGui::TreePop();
            }

            // Log a custom message
            static char str[256] = "Example text";
            ImGui::InputText("##On", str, 256);
            ImGui::SameLine();
            if (ImGui::Button("Log"))
                AX_CORE_TRACE(LogChannel::Debug, "{0}", str);
            ImGui::SameLine();
            HelpMarker("Log a custom message. 256 characters max");
        }
    }
} // namespace Axle::Debug
