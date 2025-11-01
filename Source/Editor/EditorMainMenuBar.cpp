#include "Editor.h"

#include "Core/engine.h"
#include "imgui.h"

void Editor::DrawMainMenuBar(App* app) 
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New")) 
            { 

            }

            if (ImGui::MenuItem("Open", "Ctrl+O")) 
            { 

            }

            if (ImGui::MenuItem("Save", "Ctrl+S")) 
            { 

            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit")) 
            { 
                app->isRunning = false;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About")) 
            { 

            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
