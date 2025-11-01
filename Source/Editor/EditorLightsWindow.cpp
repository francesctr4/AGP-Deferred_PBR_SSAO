#include "Editor.h"

#include "Core/engine.h"
#include "imgui.h"

#include "ImGuiUtils.inl"

void Editor::DrawLightsWindow(App* app)
{
    static bool lightsDirty = false;
    static bool lightListDirty = false;
    static bool showGridSettings = false;
    static bool showDefaultLights = true;

    ImGui::Begin("Lights Configuration");

    // ====== Section: Global Controls ======
    ImGui::BeginGroup();
    {
        ImGui::Text("Stress Test:");
        ImGui::SameLine();

        if (ImGuiUtils::ToggleButton("Grid Lights", &app->gridLightsEnabled))
        {
            lightsDirty = true;
            lightListDirty = true;
        }

        ImGui::SameLine(0, 20);
        ImGui::Text("Debug Geometry:");
        ImGui::SameLine();
        ImGuiUtils::ToggleButton("Show Debug##Lights", &app->enableLightDebug);
    }
    ImGui::EndGroup();

    // ====== Section: Grid Light Controls ======
    if (app->gridLightsEnabled)
    {
        ImGui::Spacing();

        // Grid Configuration Section
        if (ImGui::TreeNodeEx("Grid Configuration", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));

            // Grid Dimensions
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Layout:");
            ImGui::BeginTable("##GridDimensions", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit);
            {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 90);
                ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Columns (X)");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragInt("##Columns", &app->gridConfig.gridSizeX, 1, 1, 1000, "%d", ImGuiSliderFlags_AlwaysClamp);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Rows (Z)");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragInt("##Rows", &app->gridConfig.gridSizeZ, 1, 1, 1000, "%d", ImGuiSliderFlags_AlwaysClamp);

                ImGui::EndTable();
            }

            // Position Ranges
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Positioning:");
            ImGui::BeginTable("##PositionRanges", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit);
            {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 90);
                ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("X Range");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloatRange2("##XRange", &app->gridConfig.minX, &app->gridConfig.maxX, 0.5f, -100.0f, 100.0f, "Min: %.1f", "Max: %.1f");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Z Range");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloatRange2("##ZRange", &app->gridConfig.minZ, &app->gridConfig.maxZ, 0.5f, -100.0f, 100.0f, "Min: %.1f", "Max: %.1f");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Y Position");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloat("##YPos", &app->gridConfig.yPos, 0.1f, -10.0f, 100.0f, "%.1f m");

                ImGui::EndTable();
            }

            // Status and regeneration
            ImGui::Spacing();
            ImGui::TextDisabled("Total Grid Lights: %d", app->gridConfig.gridSizeX * app->gridConfig.gridSizeZ);

            ImGui::Spacing();
            if (ImGui::Button("Regenerate Grid", ImVec2(-FLT_MIN, 24)))
            {
                app->UpdateLightList();
                app->CreateLights();
                app->UpdateLights();
            }

            ImGui::PopStyleVar(2);
            ImGui::TreePop();
        }

        // Grid Light Settings Section
        ImGui::Spacing();
        if (ImGui::TreeNodeEx("Light Properties", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));

            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Attenuation:");
            bool attenuationChanged = false;

            ImGui::BeginTable("##AttenuationSettings", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit);
            {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 90);
                ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                auto AddAttenuationControl = [&](const char* label, float* value, const char* format, float speed) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", label);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    return ImGui::DragFloat(("##" + std::string(label)).c_str(), value, speed, 0.0f, 5.0f, format);
                    };

                attenuationChanged |= AddAttenuationControl("Constant", &app->gridConfig.gridLightConstant, "%.2f", 0.01f);
                attenuationChanged |= AddAttenuationControl("Linear", &app->gridConfig.gridLightLinear, "%.3f", 0.001f);
                attenuationChanged |= AddAttenuationControl("Quadratic", &app->gridConfig.gridLightQuadratic, "%.4f", 0.0001f);
                attenuationChanged |= AddAttenuationControl("Specular", &app->gridConfig.gridLightSpecularStrength, "%.4f", 0.0001f);

                ImGui::EndTable();
            }

            if (attenuationChanged)
            {
                for (auto& light : app->gridLights)
                {
                    light.constant = app->gridConfig.gridLightConstant;
                    light.linear = app->gridConfig.gridLightLinear;
                    light.quadratic = app->gridConfig.gridLightQuadratic;
                    light.specularStrength = app->gridConfig.gridLightSpecularStrength;
                }
                lightsDirty = true;
            }

            ImGui::PopStyleVar(2);
            ImGui::TreePop();
        }
    }

    // ====== Section: Default Lights ======
    if (!app->gridLightsEnabled)
    {
        ImGui::Spacing();
        if (ImGui::TreeNodeEx("Scene Lights", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));

            // Light counter
            ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Active Lights: %d", static_cast<int>(app->lights.size()));
            ImGui::Separator();
            ImGui::Spacing();

            for (auto& light : app->defaultLights)
            {
                ImGui::PushID(&light);
                bool lightChanged = false;

                // Light type and header
                const char* lightTypeName = light.type == LightType::LightType_Directional ? 
                    "Directional Light" : "Point Light";

                if (ImGui::TreeNodeEx(lightTypeName, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    // Main properties table
                    ImGui::BeginTable("##LightProps", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit);
                    {
                        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 90);
                        ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                        // Color picker
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Color");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(100);
                        lightChanged |= ImGui::ColorEdit3("##Color", &light.color[0],
                            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

                        // Position/Direction based on type
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),
                            light.type == LightType::LightType_Directional ? "Direction" : "Position");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        if (light.type == LightType::LightType_Directional) {
                            lightChanged |= ImGui::DragFloat3("##Direction", &light.direction[0], 0.01f, -1.0f, 1.0f, "%.2f");
                        }
                        else {
                            lightChanged |= ImGui::DragFloat3("##Position", &light.position[0], 0.1f, -FLT_MAX, FLT_MAX, "%.1f");
                        }

                        ImGui::EndTable();
                    }

                    // Attenuation settings (only for point lights)
                    if (light.type == LightType::LightType_Point)
                    {
                        ImGui::Spacing();
                        if (ImGui::TreeNodeEx("Advanced Properties", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::BeginTable("##Attenuation", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit);
                            {
                                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 90);
                                ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                                auto AddLightControl = [&](const char* label, float* value, const char* format, float speed) {
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::Text("%s", label);
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::SetNextItemWidth(-FLT_MIN);
                                    return ImGui::DragFloat(("##" + std::string(label)).c_str(), value, speed, 0.0f, 5.0f, format);
                                    };

                                lightChanged |= AddLightControl("Constant", &light.constant, "%.2f", 0.01f);
                                lightChanged |= AddLightControl("Linear", &light.linear, "%.3f", 0.001f);
                                lightChanged |= AddLightControl("Quadratic", &light.quadratic, "%.4f", 0.0001f);
                                lightChanged |= AddLightControl("Specular", &light.specularStrength, "%.4f", 0.0001f);

                                ImGui::EndTable();
                            }
                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }

                if (lightChanged)
                {
                    lightsDirty = true;
                    lightListDirty = true;
                }

                ImGui::PopID();
                ImGui::Spacing();
            }

            ImGui::PopStyleVar(2);
            ImGui::TreePop();
        }
    }

    ImGui::End();

    // Batched updates
    if (lightListDirty)
    {
        app->UpdateLightList();
        lightListDirty = false;
    }

    if (lightsDirty)
    {
        app->UpdateLights(); 
        lightsDirty = false;
    }
}