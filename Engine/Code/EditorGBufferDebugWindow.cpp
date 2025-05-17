#include "Editor.h"

#include "engine.h"
#include "imgui.h"

#include "ImGuiUtils.inl"

void Editor::DrawGBufferDebugWindow(App* app)
{
    ImGui::Begin("GBuffer Debug View");
    {
        static Mode prevMode = app->mode;
        ImGui::Separator();

        // Start horizontal layout
        ImGui::BeginGroup();

        // Rendering Mode
        ImGui::Text("Rendering:");
        ImGui::SameLine();
        if (ImGuiUtils::ToggleButton("##RenderingMode", &app->useDeferredRendering)) {
            app->mode = static_cast<Mode>((app->useDeferredRendering ? 1 : 0) + (app->usePBR ? 2 : 0));
        }
        ImGui::SameLine();
        ImGui::Text("%s", app->useDeferredRendering ? "Deferred" : "Forward");
        ImGui::SameLine();
        ImGui::Spacing();  // Add some space between sections

        // Lighting Model
        ImGui::Text("Lighting:");
        ImGui::SameLine();
        if (ImGuiUtils::ToggleButton("##LightingModel", &app->usePBR)) {
            app->mode = static_cast<Mode>((app->useDeferredRendering ? 1 : 0) + (app->usePBR ? 2 : 0));
        }
        ImGui::SameLine();
        ImGui::Text("%s", app->usePBR ? "PBR" : "Blinn-Phong");
        ImGui::SameLine();
        ImGui::Spacing();  // Add some space between sections

        // Skybox
        ImGui::Text("Skybox:");
        ImGui::SameLine();
        ImGuiUtils::ToggleButton("##UseSkybox", &app->useSkybox);

        ImGui::EndGroup();

        if (app->mode != prevMode) 
        {
            app->needsReinit = true;
            prevMode = app->mode;
        }

        if (app->mode == Mode_BlinnPhong_Deferred_Rendering) 
        {
            static const char* debugModes[] =
            {
                "Final Render", "Albedo", "Normal", "Position", "View Direction", "Depth"
            };

            ImGui::Combo("Display Mode", &app->gBufferDebugMode, debugModes, IM_ARRAYSIZE(debugModes));
            ImGui::Separator();
            ImGui::Text("DEBUG KEYS -> 1: Final Render | 2: Albedo | 3: Normal | 4: Position | 5: ViewDir | 6: Depth");
        }

        if (app->mode == Mode_PBR_Deferred_Rendering)
        {
            static const char* debugModes[] =
            {
                "Final Render", "Albedo", "Normal", "Position", "View Direction", 
                "Depth", "Metallic", "Roughness", 
                "Diffuse Irradiance (IBL)", "Specular Prefilter (IBL)", "BRDF Integration (IBL)"
            };

            ImGui::Combo("Display Mode", &app->gBufferDebugMode, debugModes, IM_ARRAYSIZE(debugModes));
            ImGui::Separator();
            ImGui::Text("DEBUG KEYS -> 1: Final Render | 2: Albedo | 3: Normal | 4: Position | 5: ViewDir | 6: Depth | 7: Metallic | 8: Roughness");
        }
    }
    ImGui::End();
}
