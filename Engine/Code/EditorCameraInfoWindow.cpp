#include "Editor.h"

#include "engine.h"
#include "imgui.h"

void Editor::DrawCameraInfo(App* app)
{
    ImGui::Begin("Camera Info");
    ImGui::Text("Camera: (%f, %f, %f)", app->worldCamera.GetPosition().x, app->worldCamera.GetPosition().y, app->worldCamera.GetPosition().z);
    ImGui::Text("Camera: (%f, %f, %f)", app->worldCamera.GetTarget().x, app->worldCamera.GetTarget().y, app->worldCamera.GetTarget().z);
    ImGui::End();
}