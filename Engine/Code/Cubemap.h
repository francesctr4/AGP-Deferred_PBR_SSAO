#pragma once

#include "platform.h"
#include "openGL_types.inl"

#include <array>

class App;

class Cubemap 
{
public:

    Cubemap();
    ~Cubemap();

    static void CreateCube();
    static void RenderCube();
    static void RenderQuad();
    static void ReleaseCube();

    bool LoadFromHDR(App* app, const char* hdrPath, int size = 2048);

    // Render skybox using specified shader
    static void RenderSkybox(App* app, u32 skyboxShaderIdx, u32 cubemapIdx, const glm::mat4& view, const glm::mat4& projection);

    GLuint GetCubemapID() const { return cubemapID; }
    GLuint GetDiffuseIrradianceMap() const { return irradianceMap; }
    GLuint GetSpecularPrefilterMap() const { return prefilterMap; }
    GLuint GetBRFDlookUpTexture() const { return BRDF_LUT; }

private:

    bool LoadHDRTexture(const char* hdrPath);

    glm::mat4 captureProjection;
    std::array<glm::mat4, 6> captureViews;

    GLuint cubemapID;
    GLuint irradianceMap;
    GLuint prefilterMap;
    GLuint BRDF_LUT;
    GLuint hdrTextureID;
    GLuint captureFBO;
    GLuint captureRBO;

    int size = 0;

    static GLuint cubeVAO;
    static GLuint cubeVBO;

    static GLuint quadVAO;
    static GLuint quadVBO;

    static bool cubeInitialized;

    void GenerateDiffuseIrradianceMap(Program& irradianceShader);
    void GenerateSpecularPrefilterMap(Program& prefilterShader);
    void GenerateBRDFIntegrationLUT(Program& brdfShader);

    void ConvertHDRToCubemap(Program& conversionShader);

    void CreateCubemapObject(GLuint& textureID);

    void SetTextureParameters(GLenum target);

    void ReleaseResources();
};