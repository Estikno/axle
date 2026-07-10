#include "axpch.hpp"

#include "Inspector.hpp"

#include <imgui.h>
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
        ImGui::SetNextWindowSize(ImVec2(300, 400));
        if (!ImGui::Begin(title, open)) {
            ImGui::End();
            return;
        }

        for (auto& f : s_Instance->m_Headers) {
            f();
        }

        ImGui::End();
    }
} // namespace Axle::Debug
