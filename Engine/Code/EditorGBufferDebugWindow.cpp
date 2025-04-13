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
        ImGui::Text("Rendering Mode:");

        ImGui::Text("Forward");
        ImGui::SameLine();
        ImGuiUtils::ToggleButton("Rendering Mode", (bool*)&app->mode);
        ImGui::SameLine();
        ImGui::Text("Deferred");

        if (app->mode != prevMode) {
            app->needsReinit = true;
            prevMode = app->mode;
        }

        static const char* debugModes[] = {
            "Final Render", "Albedo", "Normal", "Position",
            "View Direction", "Depth"
        };

        if (app->mode == Mode_Deferred_Rendering) {
            ImGui::Combo("Display Mode", &app->gBufferDebugMode,
                debugModes, IM_ARRAYSIZE(debugModes));
            ImGui::Separator();
            ImGui::Text("DEBUG KEYS -> 1: Final Render | 2: Albedo | 3: Normal | 4: Position | 5: ViewDir | 6: Depth");
        }
    }
    ImGui::End();
}
