#include "Editor.h"

#include "engine.h"
#include "imgui.h"

void Editor::DrawInfoWindow(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
    ImGui::Text(app->mOpenGLInfo.c_str());
    ImGui::End();
}