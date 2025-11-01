#include "Editor.h"

#include "Core/engine.h"
#include "imgui.h"

void Editor::DrawShaderErrorsWindow(App* app)
{
    ImGui::Begin("Shader Errors", &app->showShaderErrors);
    {
        ImGui::SameLine();
        ImGui::Text("%d error(s)", static_cast<int>(app->shaderErrors.size()));

        ImGui::BeginChild("ErrorScroll");
        for (const auto& error : app->shaderErrors) {
            ImGui::TextUnformatted(error.c_str());
            ImGui::Separator();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}