#pragma once

class App;

namespace Editor 
{
    void DrawDockspace(App* app);
    void DrawMainMenuBar(App* app);
    void DrawInfoWindow(App* app);
    void DrawLightsWindow(App* app);
    void DrawShaderErrorsWindow(App* app);
    void DrawGBufferDebugWindow(App* app);

    inline void Draw(App* app)
    {
        DrawDockspace(app);
        DrawMainMenuBar(app);
        DrawInfoWindow(app);
        DrawLightsWindow(app);
        DrawShaderErrorsWindow(app);
        DrawGBufferDebugWindow(app);
    }
}