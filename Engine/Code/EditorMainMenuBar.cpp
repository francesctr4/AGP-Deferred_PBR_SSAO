#include "Editor.h"

#include "engine.h"
#include "imgui.h"

void Editor::DrawMainMenuBar(App* app) 
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New")) { /* Handle new */ }
            if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Handle open */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Handle save */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { app->isRunning = false; }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About")) { /* Show about dialog */ }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
