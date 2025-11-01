#include "Editor.h"

#include "engine.h"
#include "imgui.h"

#include "ImGuiUtils.inl"

void Editor::DrawSSAOconfigWindow(App* app)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);

    ImGui::Begin("SSAO Settings");

    // Header with toggle
    ImGui::AlignTextToFramePadding();
    ImGui::Text("SSAO Status:");
    ImGui::SameLine();

    // Colored toggle button
    ImGui::PushStyleColor(ImGuiCol_Button, app->enableSSAO ?
        ImVec4(0.2f, 0.7f, 0.2f, 1.0f) : ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, app->enableSSAO ?
        ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    ImGuiUtils::ToggleButton("##SSAO", &app->enableSSAO);
    ImGui::SameLine();
    ImGui::Text("%s", app->enableSSAO ? " ACTIVE " : " INACTIVE ");

    ImGui::PopStyleColor(2);

    ImGui::SameLine();

    ImGui::Text("AO Texture:");
    ImGui::SameLine();

    // Colored toggle button
    ImGui::PushStyleColor(ImGuiCol_Button, app->useAOtex ?
        ImVec4(0.2f, 0.7f, 0.2f, 1.0f) : ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, app->useAOtex ?
        ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    ImGuiUtils::ToggleButton("##SSAO2", &app->useAOtex);
    ImGui::SameLine();
    ImGui::Text("%s", app->useAOtex ? " ACTIVE " : " INACTIVE ");

    ImGui::PopStyleColor(2);

    if (app->enableSSAO)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Settings in a child window for better scrolling/clipping
        ImGui::BeginChild("SSAO Settings Content", ImVec2(0, 0), true,
            ImGuiWindowFlags_AlwaysVerticalScrollbar);

        // Sampling Parameters Group
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
        if (ImGui::CollapsingHeader("Sampling Parameters", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderInt("Kernel Size", &app->ssaoSettings.kernelSize, 16, 256,
                "%d", ImGuiSliderFlags_Logarithmic);

            ImGui::SliderFloat("Radius", &app->ssaoSettings.radius, 0.1f, 50.0f, "%.2f");

            ImGui::SliderFloat("Bias", &app->ssaoSettings.bias, 0.001f, 1.0f, "%.3f");
        }

        // Occlusion Settings Group
        if (ImGui::CollapsingHeader("Occlusion Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Power", &app->ssaoSettings.power, 1.0f, 100.0f, "%.1f");
        }
        ImGui::PopStyleColor();

        // Reset Section
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        static const SSAOsettings DEFAULT_SSAO = { 64, 0.5f, 0.25f, 5.0f };
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));

        if (ImGui::Button("Reset to Defaults", ImVec2(-FLT_MIN, 0)))
        {
            app->ssaoSettings = DEFAULT_SSAO;
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();
    }

    ImGui::End();
    ImGui::PopStyleVar(4);
}