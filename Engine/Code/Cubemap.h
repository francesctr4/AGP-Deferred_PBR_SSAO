#pragma once

#include "platform.h"
#include "openGL_types.inl"

class App;

class Cubemap 
{
public:

    Cubemap();
    ~Cubemap();

    void CreateCube();

    bool LoadFromHDR(App* app, const char* hdrPath, u32 conversionShaderIdx, int size = 2048);

    // Render skybox using specified shader
    void RenderSkybox(App* app, u32 skyboxShaderIdx, u32 cubemapIdx, const glm::mat4& view, const glm::mat4& projection);

    GLuint GetCubemapID() const { return cubemapID; }

private:
    GLuint cubemapID;
    GLuint hdrTextureID;
    GLuint captureFBO;
    GLuint captureRBO;

    static GLuint cubeVAO;
    static GLuint cubeVBO;
    static bool cubeInitialized;

    void ConvertHDRToCubemap(App* app, u32 conversionShaderIdx, int size);

    GLuint CreateCubemapTexture(int size);

    void SetTextureParameters(GLenum target);

    void RenderCube();

    void ReleaseResources();
};