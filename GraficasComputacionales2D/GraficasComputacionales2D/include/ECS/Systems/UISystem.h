#pragma once

// Project Headers
#include "ECS/Components/Camera.h"
#include "ECS/Components/Render.h"
#include "ECS/Components/Steering.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"
#include "Prerequisites.h" 

// ============================================================
//  ECS :: Systems/UISystem.h
//
//  UISystem — LÓGICA de los paneles de ImGui.
//
//  IMPORTANTE: este sistema contiene SOLO los ImGui::Begin/End
//  y los widgets. El ciclo de frame de ImGui
//  (ImGui_ImplXXX_NewFrame + ImGui::NewFrame() al inicio, y
//   ImGui::Render() + RenderDrawData al final) NO va aquí:
//  vive en el game loop / en tu Window.
//
//  Por eso este sistema debe registrarse EL ÚLTIMO
//  (después del RenderSystem), para que la UI quede encima.
// ============================================================

namespace ECS {

    class UISystem final : public System {
    public:
        UISystem() = default;

        void OnStart(Registry& /*registry*/) override {
            // Habilitar docking.
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            setupDarkGUIStyle();
        }

        void OnUpdate(Registry& registry, float deltaTime) override {
            ImGuiDockNodeFlags dockspaceFlags =
                ImGuiDockNodeFlags_PassthruCentralNode;

            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspaceFlags);

            DrawOutliner(registry);
            DrawDetails(registry);
        }

        void DrawOutliner(Registry& registry) {
            ImGui::Begin("Outliner");
            {
                registry.GetView<ECS::Transform>().Each(
                    [this](ECS::EntityId id, ECS::Transform& /*transform*/) {
                        const std::string label = "Entity " + std::to_string(id);
                        const bool isSelected = (id == selectedEntity);

                        if (ImGui::Selectable(label.c_str(), isSelected)) {
                            selectedEntity = id;
                        }
                    });
            }
            ImGui::End();
        }

        void DrawDetails(Registry& registry) {
            ImGui::Begin("Details");
            {
                if (selectedEntity != ECS::NULL_ENTITY &&
                    registry.IsAlive(selectedEntity)) {

                    ImGui::Text("Entity %llu",
                        static_cast<unsigned long long>(selectedEntity));
                    ImGui::Separator();

                    if (auto* t = registry.TryGetComponent<ECS::Transform>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Transform",
                            ImGuiTreeNodeFlags_DefaultOpen)) {
                            vec2Control("Position", &t->position.x, 0, 120.0f);
                            vec2Control("Rotation", &t->rotation, 1.f, 120.0f);
                            vec2Control("Scale", &t->scale.x, 0.01f, 120.0f);
                        }
                    }

                    if (auto* r = registry.TryGetComponent<ECS::Render>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Render",
                            ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::Checkbox("Visible", &r->visible);

                            // sf::Color es 0..255; ImGui::ColorEdit4 trabaja en 0..1.
                            float color[4] = {
                              r->fillColor.r / 255.f,
                              r->fillColor.g / 255.f,
                              r->fillColor.b / 255.f,
                              r->fillColor.a / 255.f
                            };

                            if (ImGui::ColorEdit4("Fill Color", color)) {
                                r->fillColor = sf::Color(
                                    static_cast<std::uint8_t>(color[0] * 255.f),
                                    static_cast<std::uint8_t>(color[1] * 255.f),
                                    static_cast<std::uint8_t>(color[2] * 255.f),
                                    static_cast<std::uint8_t>(color[3] * 255.f)
                                );
                            }
                        }
                    }

                    if (auto* cam = registry.TryGetComponent<ECS::Camera>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Camera",
                            ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::Checkbox("Active", &cam->active);
                            ImGui::DragFloat("Zoom", &cam->zoom, 0.01f, 0.05f, 10.f);
                            ImGui::DragFloat("Follow Speed", &cam->followSpeed, 0.1f, 0.f, 50.f);

                            if (cam->followTarget == ECS::NULL_ENTITY) {
                                ImGui::Text("Follow Target: (ninguno)");
                            }
                            else {
                                ImGui::Text("Follow Target: %llu",
                                    static_cast<unsigned long long>(cam->followTarget));
                            }
                        }
                    }

                    if (auto* steering = registry.TryGetComponent<ECS::Steering>(selectedEntity)) {
                        if (ImGui::CollapsingHeader("Steering Behavior",
                            ImGuiTreeNodeFlags_DefaultOpen)) {

                            // Menú desplegable para elegir el comportamiento (ACTUALIZADO)
                            const char* items[] = {
                              "None", "Seek", "Flee", "Arrive",
                              "Wander", "Pursuit", "ObstacleAvoidance"
                            };
                            int item_current = static_cast<int>(steering->currentBehavior);
                            if (ImGui::Combo("Behavior Type", &item_current, items,
                                IM_ARRAYSIZE(items))) {
                                steering->currentBehavior =
                                    static_cast<ECS::SteeringType>(item_current);
                            }

                            // Controles para variables físicas base
                            ImGui::DragFloat("Max Speed", &steering->maxSpeed,
                                1.0f, 10.f, 500.f);
                            ImGui::DragFloat("Max Force", &steering->maxForce,
                                0.5f, 1.f, 100.f);
                            ImGui::DragFloat("Mass", &steering->mass, 0.1f, 0.1f, 10.f);

                            // Mostrar el radio de frenado solo si el comportamiento es Arrive
                            if (steering->currentBehavior == ECS::SteeringType::Arrive) {
                                ImGui::DragFloat("Slowing Radius", &steering->slowingRadius,
                                    1.0f, 10.f, 300.f);
                            }

                            // Mostrar controles exclusivos solo si el comportamiento es Wander
                            if (steering->currentBehavior == ECS::SteeringType::Wander) {
                                ImGui::DragFloat("Wander Radius", &steering->wanderRadius,
                                    1.0f, 10.f, 200.f);
                                ImGui::DragFloat("Wander Distance", &steering->wanderDistance,
                                    1.0f, 10.f, 300.f);
                                ImGui::DragFloat("Wander Jitter", &steering->wanderJitter,
                                    0.05f, 0.1f, 2.0f);
                            }
                        }
                    }

                }
                else {
                    ImGui::TextDisabled("Selecciona una entidad en el outliner.");
                }
            }
            ImGui::End();
        }

        void setupDarkGUIStyle() {
            ImGuiStyle& style = ImGui::GetStyle();

            style.Alpha = 1.0f;
            style.DisabledAlpha = 1.0f;
            style.WindowPadding = ImVec2(12.0f, 12.0f);
            style.WindowRounding = 0.0f;
            style.WindowBorderSize = 0.0f;
            style.WindowMinSize = ImVec2(20.0f, 20.0f);
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_None;
            style.ChildRounding = 0.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupRounding = 0.0f;
            style.PopupBorderSize = 1.0f;
            style.FramePadding = ImVec2(6.0f, 6.0f);
            style.FrameRounding = 0.0f;
            style.FrameBorderSize = 0.0f;
            style.ItemSpacing = ImVec2(12.0f, 6.0f);
            style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
            style.CellPadding = ImVec2(12.0f, 6.0f);
            style.IndentSpacing = 20.0f;
            style.ColumnsMinSpacing = 6.0f;
            style.ScrollbarSize = 12.0f;
            style.ScrollbarRounding = 0.0f;
            style.GrabMinSize = 12.0f;
            style.GrabRounding = 0.0f;
            style.TabRounding = 0.0f;
            style.TabBorderSize = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] =
                ImVec4(0.2745f, 0.3176f, 0.4509f, 1.0f);

            style.Colors[ImGuiCol_WindowBg] =
                ImVec4(0.0784f, 0.0862f, 0.1019f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] =
                ImVec4(0.0784f, 0.0862f, 0.1019f, 1.0f);
            style.Colors[ImGuiCol_PopupBg] =
                ImVec4(0.0784f, 0.0862f, 0.1019f, 1.0f);

            style.Colors[ImGuiCol_Border] =
                ImVec4(0.1568f, 0.1686f, 0.1921f, 1.0f);
            style.Colors[ImGuiCol_BorderShadow] =
                ImVec4(0.0784f, 0.0862f, 0.1019f, 1.0f);

            style.Colors[ImGuiCol_FrameBg] =
                ImVec4(0.1176f, 0.1333f, 0.1490f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] =
                ImVec4(0.1568f, 0.1686f, 0.1921f, 1.0f);
            style.Colors[ImGuiCol_FrameBgActive] =
                ImVec4(0.2352f, 0.2156f, 0.5960f, 1.0f);

            style.Colors[ImGuiCol_TitleBg] =
                ImVec4(0.0470f, 0.0549f, 0.0705f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] =
                ImVec4(0.0470f, 0.0549f, 0.0705f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] =
                ImVec4(0.0784f, 0.0862f, 0.1019f, 1.0f);

            style.Colors[ImGuiCol_MenuBarBg] =
                ImVec4(0.0980f, 0.1058f, 0.1215f, 1.0f);

            style.Colors[ImGuiCol_ScrollbarBg] =
                ImVec4(0.0470f, 0.0549f, 0.0705f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] =
                ImVec4(0.1176f, 0.1333f, 0.1490f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] =
                ImVec4(0.1568f, 0.1686f, 0.1921f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] =
                ImVec4(0.1176f, 0.1333f, 0.1490f, 1.0f);

            style.Colors[ImGuiCol_CheckMark] =
                ImVec4(0.4980f, 0.5137f, 1.0f, 1.0f);

            style.Colors[ImGuiCol_SliderGrab] =
                ImVec4(0.4980f, 0.5137f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] =
                ImVec4(0.5372f, 0.5529f, 1.0f, 1.0f);

            style.Colors[ImGuiCol_Button] =
                ImVec4(0.1176f, 0.1333f, 0.1490f, 1.0f);
            style.Colors[ImGuiCol_ButtonHovered] =
                ImVec4(0.1960f, 0.1764f, 0.5450f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] =
                ImVec4(0.2352f, 0.2156f, 0.5960f, 1.0f);

            style.Colors[ImGuiCol_Header] =
                ImVec4(0.1176f, 0.1333f, 0.1490f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] =
                ImVec4(0.1960f, 0.1764f, 0.5450f, 1.0f);
            style.Colors[ImGuiCol_HeaderActive] =
                ImVec4(0.2352f, 0.2156f, 0.5960f, 1.0f);

            style.Colors[ImGuiCol_Separator] =
                ImVec4(0.1568f, 0.1843f, 0.2509f, 1.0f);
            style.Colors[ImGuiCol_SeparatorHovered] =
                ImVec4(0.1568f, 0.1843f, 0.2509f, 1.0f);
            style.Colors[ImGuiCol_SeparatorActive] =
                ImVec4(0.1568f, 0.1843f, 0.2509f, 1.0f);

            style.Colors[ImGuiCol_ResizeGrip] =
                ImVec4(0.1176f, 0.1333f, 0.1490f, 1.0f);
            style.Colors[ImGuiCol_ResizeGripHovered] =
                ImVec4(0.1960f, 0.1764f, 0.5450f, 1.0f);
            style.Colors[ImGuiCol_ResizeGripActive] =
                ImVec4(0.2352f, 0.2156f, 0.5960f, 1.0f);

            style.Colors[ImGuiCol_Tab] =
                ImVec4(0.0470f, 0.0549f, 0.0705f, 1.0f);
            style.Colors[ImGuiCol_TabHovered] =
                ImVec4(0.1176f, 0.1333f, 0.1490f, 1.0f);
            style.Colors[ImGuiCol_TabActive] =
                ImVec4(0.0980f, 0.1058f, 0.1215f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] =
                ImVec4(0.0470f, 0.0549f, 0.0705f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocusedActive] =
                ImVec4(0.0784f, 0.0862f, 0.1019f, 1.0f);

            style.Colors[ImGuiCol_PlotLines] =
                ImVec4(0.5215f, 0.6000f, 0.7019f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] =
                ImVec4(0.0392f, 0.9803f, 0.9803f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] =
                ImVec4(1.0f, 0.2901f, 0.5960f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] =
                ImVec4(0.9960f, 0.4745f, 0.6980f, 1.0f);

            style.Colors[ImGuiCol_TableHeaderBg] =
                ImVec4(0.0470f, 0.0549f, 0.0705f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] =
                ImVec4(0.0470f, 0.0549f, 0.0705f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] =
                ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

            style.Colors[ImGuiCol_TableRowBg] =
                ImVec4(0.1176f, 0.1333f, 0.1490f, 1.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] =
                ImVec4(0.0980f, 0.1058f, 0.1215f, 1.0f);

            style.Colors[ImGuiCol_TextSelectedBg] =
                ImVec4(0.2352f, 0.2156f, 0.5960f, 1.0f);
            style.Colors[ImGuiCol_DragDropTarget] =
                ImVec4(0.4980f, 0.5137f, 1.0f, 1.0f);

            style.Colors[ImGuiCol_NavHighlight] =
                ImVec4(0.4980f, 0.5137f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] =
                ImVec4(0.4980f, 0.5137f, 1.0f, 1.0f);

            style.Colors[ImGuiCol_NavWindowingDimBg] =
                ImVec4(0.1960f, 0.1764f, 0.5450f, 0.5019f);
            style.Colors[ImGuiCol_ModalWindowDimBg] =
                ImVec4(0.1960f, 0.1764f, 0.5450f, 0.5019f);
        }

        void vec2Control(const std::string& label, float* values,
            float resetValues, float columnWidth) {
            ImGuiIO& io = ImGui::GetIO();
            auto boldFont = io.Fonts->Fonts[0];

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(label.c_str());
            ImGui::NextColumn();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

            float lineHeight = ImGui::GetFontSize() +
                ImGui::GetStyle().FramePadding.y * 2.0f;
            ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

            float fullWidth = ImGui::CalcItemWidth();
            float itemWidth = fullWidth / 3.0f;

            // --- X Control ---
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            ImGui::PushFont(boldFont);
            if (ImGui::Button("X", buttonSize)) {
                values[0] = resetValues;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::PushItemWidth(itemWidth);
            ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            // --- Y Control ---
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
            ImGui::PushFont(boldFont);
            if (ImGui::Button("Y", buttonSize)) {
                values[1] = resetValues;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::PushItemWidth(itemWidth);
            ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PopStyleVar();
            ImGui::Columns(1);
            ImGui::PopID();
        }

    private:
        // Entidad seleccionada en el outliner (NULL_ENTITY = ninguna).
        ECS::EntityId selectedEntity = ECS::NULL_ENTITY;
    };

} // namespace ECS