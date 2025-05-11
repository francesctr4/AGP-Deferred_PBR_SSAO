#pragma once

#include "platform.h"
#include "openGL_types.inl"

class App;

class Cubemap {
public:
    Cubemap();

    ~Cubemap();

    // Load HDR environment map and convert to cubemap
    bool LoadFromHDR(App* app, const char* hdrPath, u32 conversionShaderIdx, int size = 512);

    // Render skybox using specified shader
    void RenderSkybox(App* app, u32 skyboxShaderIdx, const glm::mat4& view, const glm::mat4& projection);

    GLuint GetCubemapID() const { return cubemapID; }

private:
    GLuint cubemapID;
    GLuint hdrTextureID;
    GLuint captureFBO;
    GLuint captureRBO;

    static GLuint cubeVAO;
    static GLuint cubeVBO;
    static bool cubeInitialized;

    void InitializeCube();

    void ConvertHDRToCubemap(App* app, u32 conversionShaderIdx, int size);

    GLuint CreateCubemapTexture(int size);

    void SetTextureParameters(GLenum target);

    void RenderCube();

    void ReleaseResources();
};