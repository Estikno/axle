#pragma once

#include "axpch.hpp"

#include "imgui.h"
#include "Core/Logger/Log.hpp"
#include "Core/Types.hpp"
#include "ECS/ECS.hpp"

namespace Axle::Debug {
    struct ECSEditor {
        bool m_Open = true;

        void Draw(const char* title, bool* p_open) {
            if (!ImGui::Begin(title, p_open)) {
                ImGui::End();
                return;
            }

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Close Console"))
                    *p_open = false;
                ImGui::EndPopup();
            }

            DrawEntityList();
            ImGui::SameLine();
            DrawComponentDetails();

            ImGui::End();
        }

        void DrawEntityList() {
            ImGui::BeginChild("##EntityList", ImVec2(160, 0), true);
            ImGui::Text("Entities");
            ImGui::Separator();

            for (EntityID id = 0; id < MAX_ENTITIES; id++) {
                if (!ECS::GetInstance().IsAlive(id))
                    continue;

                char label[32];
                snprintf(label, sizeof(label), "Entity %u", id);

                bool selected = (m_SelectedEntity == id);
                if (ImGui::Selectable(label, selected))
                    m_SelectedEntity = id;
            }

            ImGui::EndChild();
        }

        void DrawComponentDetails() {
            ECS& m_ECS = ECS::GetInstance();
            ImGui::BeginChild("##ComponentDetails", ImVec2(0, 0), true);

            if (m_SelectedEntity == INVALID_ENTITY) {
                ImGui::TextDisabled("No entity selected.");
                ImGui::EndChild();
                return;
            }

            ImGui::Text("Entity %u", m_SelectedEntity);
            ImGui::Separator();

            // Delete entity button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button("Delete Entity")) {
                m_ECS.DeleteEntity(m_SelectedEntity);
                m_SelectedEntity = INVALID_ENTITY;
                ImGui::PopStyleColor();
                ImGui::EndChild();
                return;
            }
            ImGui::PopStyleColor();

            ImGui::Spacing();

            // Draw each registered component if this entity has it
            for (auto& [typeID, descriptor] : m_ECS.GetDescriptors()) {
                auto it = m_ECS.GetComponentArraysTEST().find(typeID);
                if (it == m_ECS.GetComponentArraysTEST().end())
                    continue;
                if (!it->second->Has(m_SelectedEntity))
                    continue;

                // Colored header per component
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 0.7f, 1.0f));

                bool open = ImGui::CollapsingHeader(descriptor.name.c_str());
                ImGui::PopStyleColor(2);

                if (open) {
                    ImGui::PushID(static_cast<int>(typeID)); // avoid ImGui ID collisions
                    ImGui::Indent();

                    void* ptr = it->second->GetRaw(m_SelectedEntity);
                    descriptor.drawImGui(ptr);

                    ImGui::Unindent();
                    ImGui::PopID();
                }

                ImGui::Spacing();
            }

            ImGui::EndChild();
        }

        EntityID m_SelectedEntity = INVALID_ENTITY;
    };
} // namespace Axle::Debug
