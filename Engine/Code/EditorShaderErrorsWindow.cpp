#include "Editor.h"

#include "engine.h"
#include "imgui.h"

void Editor::DrawShaderErrorsWindow(App* app)
{
    // ... rest of your GUI code (Error display, GBuffer Debug View) remains the same ...
    // Error display window
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