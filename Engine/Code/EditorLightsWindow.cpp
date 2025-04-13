#include "Editor.h"

#include "engine.h"
#include "imgui.h"

#include "ImGuiUtils.inl"

void Editor::DrawLightsWindow(App* app)
{
    // Local dirty flags for batched updates
    bool lightsDirty = false;
    bool lightListDirty = false;

    ImGui::Begin("Lights");
    ImGui::Text("Stress Test");
    ImGui::SameLine();
    static bool prevGridState = app->gridLightsEnabled;
    ImGuiUtils::ToggleButton("Idx", (bool*)&app->gridLightsEnabled);

    ImGui::Text("Debug Geometry");
    ImGui::SameLine();
    ImGuiUtils::ToggleButton("DebugGeometryIdx", (bool*)&app->enableLightDebug);

    // Handle grid light toggle
    if (prevGridState != app->gridLightsEnabled) {
        lightListDirty = true;
        prevGridState = app->gridLightsEnabled;
    }

    ImGui::Text("Quantity of Lights: %d", static_cast<int>(app->lights.size()));
    ImGui::Separator();

    // Grid light attenuation controls
    if (app->gridLightsEnabled) {
        bool attenuationChanged = false;
        attenuationChanged |= ImGui::DragFloat("Constant", &app->gridLightConstant, 0.01f, 0.0f, 5.0f);
        attenuationChanged |= ImGui::DragFloat("Linear", &app->gridLightLinear, 0.001f, 0.0f, 5.0f);
        attenuationChanged |= ImGui::DragFloat("Quadratic", &app->gridLightQuadratic, 0.0001f, 0.0f, 5.0f);

        if (attenuationChanged) {
            for (auto& light : app->gridLights) {
                light.constant = app->gridLightConstant;
                light.linear = app->gridLightLinear;
                light.quadratic = app->gridLightQuadratic;
            }
            lightsDirty = true;
        }
    }

    // Default light controls
    for (auto& light : app->defaultLights) {
        ImGui::PushID(&light);
        bool lightChanged = false;

        // Color control
        float color[3] = { light.color.x, light.color.y, light.color.z };
        if (ImGui::ColorEdit3("Color", color)) {
            light.color = glm::vec3(color[0], color[1], color[2]);
            lightChanged = true;
        }

        // Direction control
        float direction[3] = { light.direction.x, light.direction.y, light.direction.z };
        if (ImGui::DragFloat3("Direction", direction, 0.01f, -1.0f, 1.0f)) {
            light.direction = glm::vec3(direction[0], direction[1], direction[2]);
            lightChanged = true;
        }

        // Position control
        float position[3] = { light.position.x, light.position.y, light.position.z };
        if (ImGui::DragFloat3("Position", position, 0.1f)) {
            light.position = glm::vec3(position[0], position[1], position[2]);
            lightChanged = true;
        }

        // Attenuation controls
        lightChanged |= ImGui::DragFloat("Constant", &light.constant, 0.01f, 0.0f, 1.0f);
        lightChanged |= ImGui::DragFloat("Linear", &light.linear, 0.001f, 0.0f, 0.1f);
        lightChanged |= ImGui::DragFloat("Quadratic", &light.quadratic, 0.0001f, 0.0f, 0.01f);
        lightChanged |= ImGui::DragFloat("Specular", &light.specularStrength, 0.01f, 0.0f, 1.0f);

        if (lightChanged) {
            lightsDirty = true;
            // Force light list update when default lights change
            lightListDirty = true;
        }

        ImGui::PopID();
        ImGui::Separator();
    }

    ImGui::End();

    // Batched updates at the end
    if (lightListDirty) 
    {
        app->UpdateLightList();
        lightsDirty = true; // Ensure shader update after list changes
    }

    if (lightsDirty) 
    {
        app->UpdateLights();
    }
}