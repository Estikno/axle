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
            ImGui::BeginChild("##EntityList",
                              ImVec2(200, 0),
                              ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened);
            ImGui::Text("Entities");
            ImGui::SameLine();
            if (ImGui::Button("New Entity")) {
                m_SelectedEntity = ECS::GetInstance().CreateEntity().GetLastCreatedEntity();
            }
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

            ImGui::Text("Entity %lu", m_SelectedEntity);
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
            ImGui::SameLine();
            DrawAddComponentPopup();

            ImGui::Spacing();

            // Draw each registered component if this entity has it
            for (auto& [typeID, descriptor] : m_ECS.GetDescriptors()) {
                auto it = m_ECS.GetComponentArraysTEST().find(typeID);
                if (it == m_ECS.GetComponentArraysTEST().end())
                    continue;
                if (!it->second->Has(m_SelectedEntity))
                    continue;

                ImGui::PushID(static_cast<int>(typeID)); // avoid ImGui ID collisions

                // Colored header per component
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 0.7f, 1.0f));

                bool open = ImGui::CollapsingHeader(descriptor.name.c_str(), ImGuiTreeNodeFlags_AllowOverlap);
                ImGui::PopStyleColor(2);

                // X button on the same line
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, .1f, .1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                bool removed = ImGui::SmallButton("X");
                ImGui::PopStyleColor(2);

                if (removed) {
                    descriptor.removeFromEntity(m_SelectedEntity);
                    ImGui::PopID();
                    continue;
                }

                if (open) {
                    ImGui::Indent();

                    void* ptr = it->second->GetRaw(m_SelectedEntity);
                    descriptor.drawImGui(ptr);

                    ImGui::Unindent();
                }

                ImGui::Spacing();
                ImGui::PopID();
            }

            ImGui::EndChild();
        }

        void DrawAddComponentPopup() {
            ECS& m_ECS = ECS::GetInstance();

            if (ImGui::Button("Add Component"))
                ImGui::OpenPopup("##AddComponent");

            if (ImGui::BeginPopup("##AddComponent")) {
                ImGui::Text("Components");
                ImGui::Separator();

                for (auto& [typeID, descriptor] : m_ECS.GetDescriptors()) {
                    // Only show components the entity doesn't already have
                    auto it = m_ECS.GetComponentArraysTEST().find(typeID);
                    if (it != m_ECS.GetComponentArraysTEST().end() && it->second->Has(m_SelectedEntity))
                        continue;

                    if (ImGui::Selectable(descriptor.name.c_str())) {
                        descriptor.addToEntity(m_SelectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::EndPopup();
            }
        }

        EntityID m_SelectedEntity = INVALID_ENTITY;
    };
} // namespace Axle::Debug
