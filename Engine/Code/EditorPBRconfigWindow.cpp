#include "Editor.h"

#include "engine.h"
#include "imgui.h"

#include "ImGuiUtils.inl"

void Editor::DrawPBRconfigWindow(App* app)
{
    ImGui::Begin("PBR Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Window styling
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 12));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));

    // IBL Section
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.1f, 1.0f), "Image Based Lighting");
    ImGui::SameLine();
    ImGuiUtils::ToggleButton("##IBL", &app->enableIBL);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
        app->enableIBL ? "Enabled (HDRI-based lighting)" : "Disabled");
    ImGui::Separator();

    // Material Controls Section
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.1f, 1.0f), "Material Properties");
    ImGui::Separator();

    ImGui::BeginGroup();
    {
        // Metallic control with percentage
        ImGui::Text("Metallic Influence:");
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##Metallic", &app->metallicInfluence, 0.0f, 1.0f, "%.2f");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%.0f%%", app->metallicInfluence * 100);

        // Roughness control with percentage
        ImGui::Text("Roughness Influence:");
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##Roughness", &app->roughnessInfluence, 0.0f, 1.0f, "%.2f");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%.0f%%", app->roughnessInfluence * 100);
    }
    ImGui::EndGroup();

    // Tooltips
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Control texture influence on material properties");
        ImGui::BulletText("Metallic: Controls metal reflection intensity");
        ImGui::BulletText("Roughness: Affects surface micro-detail scattering");
        ImGui::EndTooltip();
    }

    // Style cleanup
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::End();
}