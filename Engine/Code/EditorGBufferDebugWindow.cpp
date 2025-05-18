#include "Editor.h"

#include "engine.h"
#include "imgui.h"

#include "ImGuiUtils.inl"

void Editor::DrawGBufferDebugWindow(App* app)
{
    ImGui::Begin("GBuffer Debug View", (bool*)0, 
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse);
    {
        static Mode prevMode = app->mode;
        ImGui::Separator();

        // Horizontal layout using a table
        ImGui::BeginTable("ModeToggles", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoBordersInBody);

        ImGui::TableSetupColumn("Rendering", ImGuiTableColumnFlags_WidthFixed, 150);
        ImGui::TableSetupColumn("Lighting", ImGuiTableColumnFlags_WidthFixed, 150);
        ImGui::TableSetupColumn("Skybox", ImGuiTableColumnFlags_WidthFixed, 100);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // Rendering Mode
        ImGui::BeginGroup();
        ImGui::Text("Rendering Mode:");
        if (ImGuiUtils::ToggleButton("##RenderingMode", &app->useDeferredRendering)) {
            app->mode = static_cast<Mode>((app->useDeferredRendering ? 1 : 0) + (app->usePBR ? 2 : 0));
        }
        ImGui::SameLine();
        ImGui::Text("%s", app->useDeferredRendering ? "Deferred" : "Forward");
        ImGui::EndGroup();

        ImGui::TableNextColumn();

        // Lighting Model
        ImGui::BeginGroup();
        ImGui::Text("Lighting Model:");
        if (ImGuiUtils::ToggleButton("##LightingModel", &app->usePBR)) {
            app->mode = static_cast<Mode>((app->useDeferredRendering ? 1 : 0) + (app->usePBR ? 2 : 0));
        }
        ImGui::SameLine();
        ImGui::Text("%s", app->usePBR ? "PBR" : "Blinn-Phong");
        ImGui::EndGroup();

        ImGui::TableNextColumn();

        // Skybox
        ImGui::BeginGroup();
        ImGui::Text("Skybox:");
        ImGuiUtils::ToggleButton("##UseSkybox", &app->useSkybox);
        ImGui::SameLine();
        ImGui::Text("%s", app->useSkybox ? "Enabled" : "Disabled");
        ImGui::EndGroup();

        ImGui::EndTable();

        if (app->mode != prevMode) 
        {
            app->needsReinit = true;
            prevMode = app->mode;
        }

        if (app->mode == Mode_BlinnPhong_Deferred_Rendering) 
        {
            static const char* debugModes[] =
            {
                "Final Render", "Albedo", "Normal", "Position", "View Direction", "Depth", "Ambient Occlusion"
            };

            ImGui::Combo("Display Mode", &app->gBufferDebugMode, debugModes, IM_ARRAYSIZE(debugModes));
            ImGui::Separator();
            // Grid-styled debug keys
            ImGui::Text("DEBUG KEYS");
            ImGui::Text("1: Final Render | 2: Albedo | 3: Normal | 4: Position | 5: ViewDir | 6: Depth | 7: SSAO");
        }

        if (app->mode == Mode_PBR_Deferred_Rendering)
        {
            static const char* debugModes[] =
            {
                "Final Render", "Albedo", "Normal", "Position", "View Direction",
                "Depth", "Ambient Occlusion", "Metallic", "Roughness",
                "Diffuse Irradiance (IBL)", "Specular Prefilter (IBL)", "BRDF Integration (IBL)"
            };

            ImGui::Combo("Display Mode", &app->gBufferDebugMode, debugModes, IM_ARRAYSIZE(debugModes));
            ImGui::Separator();

            // Grid-styled debug keys
            ImGui::Text("DEBUG KEYS");
            ImGui::Text("1: Final Render | 2: Albedo | 3: Normal | 4: Position | 5: ViewDir | 6: Depth | 7: SSAO | 8: Metallic | 9: Roughness");
        }
    }
    ImGui::End();
}