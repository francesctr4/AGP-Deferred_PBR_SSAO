#include "Editor.h"

#include "engine.h"
#include "imgui.h"

void Editor::DrawDockspace(App* app) 
{
    // Set DockSpace Invisible Window Flags
    ImGuiWindowFlags window = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // Get Window Viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Set Window Parameters
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.0f);

    // Set Window Style Parameters
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Begin DockSpace Invisible Window with the flags
    ImGui::Begin("Dockspace", 0, window);

    // Apply Window Style Parameters
    ImGui::PopStyleVar(3);

    // Create DockSpace on the invisible window
    ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    // End DockSpace Window
    ImGui::End();
}