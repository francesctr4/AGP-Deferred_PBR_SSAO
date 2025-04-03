#pragma once

#include "engine.h"

#include <imgui.h>

namespace Editor 
{
    void DrawDockspace(App* app);
    void DrawMainMenuBar(App* app);
    void DrawInfoWindow(App* app);
    void DrawLightsWindow(App* app);
    void DrawShaderErrorsWindow(App* app);
    void DrawGBufferDebugWindow(App* app);
    void DrawFBOTexturesWindow(App* app);
}