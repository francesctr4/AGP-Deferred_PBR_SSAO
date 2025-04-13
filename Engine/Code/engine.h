#ifndef _ENGINE_H_
#define _ENGINE_H_

//
// engine.h: Engine types and core functionality
//

#include "openGL_types.inl"
#include "OpenGL_Framebuffer.h"
#include "BufferManagement.h"
#include "Camera.h"
#include <vector>

class App
{
public:

    App();
    ~App();

    void Init(App* app);
    void Update(App* app);
    void Render(App* app);
    void Gui(App* app);
    void CleanUp(App* app);

    void OnResize(int width, int height);

    // ====================
    // Lighting Management
    // ====================
    void CreateLights();
    void UpdateLights(App* app);
    void UpdateLightList();
    void RenderLightDebugGeometry();

private:

    // ====================
    // Rendering Internals
    // ====================
    void CameraMovement(App* app);
    GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

    void CreateEntity(App* app, u32 modelIdx, u32 textureIdx, const glm::mat4& worldMatrix);
    void UpdateEntities(App* app);
    void RenderEntity(Entity* entity, Program& program, u32 programUniformTexture);

public:

    bool isRunning;
    f32  deltaTime;
    Input input;
    Mode mode;
    bool needsReinit;

    ivec2 displaySize;

    // ====================
    // Graphics Resources
    // ====================
    std::string mOpenGLInfo;

    // Resource pools
    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh>     meshes;
    std::vector<Model>    models;
    std::vector<Program>  programs;
    std::vector<Entity> entities;

    // ================
    // Rendering State
    // ================
    Camera worldCamera;
    Framebuffer primaryFBO;

    // Embedded geometry
    GLuint embeddedVertices;
    GLuint embeddedElements;
    GLuint vao;

    // ====================
    // Buffers and UBOs
    // ====================
    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;

    Buffer globalUBO;
    Buffer entityUBO;

    // ============
    // Lighting
    // ============
    std::vector<Light> lights;
    std::vector<Light> gridLights;
    std::vector<Light> defaultLights;

    bool gridLightsEnabled = true;
    float gridLightConstant;
    float gridLightLinear;
    float gridLightQuadratic;

    // ====================
    // Debug/UI State
    // ====================
    int gBufferDebugMode = 0;
    std::vector<std::string> shaderErrors;
    bool showShaderErrors = false;

private:

    // Texture Indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    u32 lightBlueTexIdx;
    u32 greenTexIdx;
    u32 purpleTexIdx;
    u32 blueTexIdx;
    u32 lightGreenTexIdx;
    u32 orangeTexIdx;

    // Model Indices
    u32 patrickIdx;
    u32 planeIdx;
    u32 coneIdx;
    u32 cubeIdx;
    u32 cylinderIdx;
    u32 sphereIdx;
    u32 torusIdx;

    // Shader Program Indices
    u32 deferredRenderQuadProgramIdx;
    u32 deferredRenderGeometryProgramIdx;
    u32 forwardRenderProgramIdx;
    u32 pointLightSphereProgramIdx;

    // Uniform locations
    u32 deferredRenderProgramUniformTexture;
    u32 forwardRenderProgramUniformTexture;

    GLuint programUniformDebugMode;

};

#endif // _ENGINE_H_