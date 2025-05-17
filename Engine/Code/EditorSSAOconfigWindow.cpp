#include "Editor.h"

#include "engine.h"
#include "imgui.h"

void Editor::DrawSSAOconfigWindow(App* app)
{
    ImGui::Begin("SSAO Settings");

    // Kernel size slider (typically between 16-256 samples)
    ImGui::SliderInt("Kernel Size", &app->ssaoSettings.kernelSize, 16, 256);

    // Radius slider for sampling area
    ImGui::SliderFloat("Radius", &app->ssaoSettings.radius, 0.1f, 50.0f, "%.2f");

    // Bias slider to prevent self-occlusion
    ImGui::SliderFloat("Bias", &app->ssaoSettings.bias, 0.001f, 1.0f, "%.3f");

    // Power slider for occlusion contrast
    ImGui::SliderFloat("Power", &app->ssaoSettings.power, 1.0f, 100.0f, "%.1f");

    // Define default values as static constants
    static const SSAOsettings DEFAULT_SSAO = {
        .kernelSize = 64,
        .radius = 0.5f,
        .bias = 0.25f,
        .power = 5.0f
    };

    // Reset button with warning color
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.5f, 0.0f, 1.0f)); // Orange
    if (ImGui::Button("Reset to Defaults")) {
        app->ssaoSettings = DEFAULT_SSAO;
    }
    ImGui::PopStyleColor();

    ImGui::End();
}