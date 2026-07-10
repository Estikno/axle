#include "axpch.hpp"

#include "Inspector.hpp"

#include <imgui.h>

namespace Axle::Debug {
    void Inspector::Draw(const char* title, bool* open) {
        ImGui::SetNextWindowSize(ImVec2(200, 600));
        if (!ImGui::Begin(title, open)) {
            ImGui::End();
            return;
        }

        ImGui::Text("Hello");
        ImGui::End();
    }
} // namespace Axle::Debug
