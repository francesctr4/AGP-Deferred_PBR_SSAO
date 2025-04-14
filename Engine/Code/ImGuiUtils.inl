#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace ImGuiUtils
{
    struct ToggleButtonConfig 
    {
        ImVec2 size = ImVec2(0, 0);          // 0 = auto-size
        float rounding = 0.5f;               // 0-1 ratio of frame height
        ImColor active_color = IM_COL32(145, 211, 68, 255);
        ImColor inactive_color = IM_COL32(218, 218, 218, 255);
        ImColor hovered_active_color = IM_COL32(165, 231, 88, 255);
        ImColor hovered_inactive_color = IM_COL32(198, 198, 198, 255);
        ImColor thumb_color = IM_COL32(255, 255, 255, 255);
        float thumb_padding = 1.5f;           // In pixels
    };

    inline bool ToggleButton(const char* str_id, bool* v, const ToggleButtonConfig& config = {})
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        // Calculate dimensions
        const float height = ImGui::GetFrameHeight();
        const float width = config.size.x > 0 ? config.size.x : height * 1.55f;
        const float radius = height * config.rounding;
        ImVec2 actual_size(config.size.x > 0 ? config.size.x : width,
            config.size.y > 0 ? config.size.y : height);

        const ImRect bb(window->DC.CursorPos, { window->DC.CursorPos.x + actual_size.x, window->DC.CursorPos.y + actual_size.y });
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, ImGui::GetID(str_id)))
            return false;

        bool clicked = false;
        if (ImGui::ButtonBehavior(bb, ImGui::GetID(str_id), nullptr, nullptr, ImGuiButtonFlags_PressedOnClick))
        {
            *v = !*v;
            clicked = true;
        }

        // Choose colors based on state
        const ImU32 col_bg = *v ?
            (ImGui::IsItemHovered() ? config.hovered_active_color : config.active_color) :
            (ImGui::IsItemHovered() ? config.hovered_inactive_color : config.inactive_color);

        // Draw background
        window->DrawList->AddRectFilled(bb.Min, bb.Max, col_bg, radius);

        // Draw thumb
        const float thumb_radius = radius - config.thumb_padding;
        const float thumb_x = *v ? (bb.Max.x - thumb_radius - config.thumb_padding) :
            (bb.Min.x + thumb_radius + config.thumb_padding);
        const ImVec2 thumb_center(thumb_x, bb.GetCenter().y);
        window->DrawList->AddCircleFilled(thumb_center, thumb_radius, config.thumb_color);

        return clicked;
    }

    inline void HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}