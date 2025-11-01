#include "Editor.h"

#include "Core/engine.h"
#include "imgui.h"

void Editor::DrawCameraInfo(App* app)
{
    ImGui::Begin("Camera Info");
    ImGui::Text("Position: (%f, %f, %f)", app->worldCamera.GetPosition().x, app->worldCamera.GetPosition().y, app->worldCamera.GetPosition().z);
    ImGui::Text("Target: (%f, %f, %f)", app->worldCamera.GetTarget().x, app->worldCamera.GetTarget().y, app->worldCamera.GetTarget().z);
    ImGui::End();
}