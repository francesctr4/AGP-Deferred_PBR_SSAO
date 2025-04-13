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
    // =====================
    // Core Application State
    // =====================
    App();
    ~App();

    bool isRunning;
    f32  deltaTime;
    Input input;
    Mode mode;
    bool needsReinit;

    ivec2 displaySize;
    void OnResize(int width, int height);

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

    // ========================
    // Shader Program Indices
    // ========================
    u32 renderQuadProgramIdx;
    u32 renderGeometryProgramIdx;
    u32 forwardRenderingProgramIdx;
    u32 lightSphereProgramIdx;

    // ====================
    // Texture Indices
    // ====================
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

    // ==================
    // Model Indices
    // ==================
    u32 patrickIdx;
    u32 planeIdx;
    u32 coneIdx;
    u32 cubeIdx;
    u32 cylinderIdx;
    u32 sphereIdx;
    u32 torusIdx;

    // ================
    // Rendering State
    // ================
    Camera worldCamera;
    Framebuffer primaryFBO;

    // Embedded geometry
    GLuint embeddedVertices;
    GLuint embeddedElements;
    GLuint vao;

    // Uniform locations
    u32 patrickProgramUniformTexture;
    u32 fwdPatrickProgramUniformTexture;
    GLuint programUniformTexture;
    GLuint programUniformDebugMode;

    // ====================
    // Buffers and UBOs
    // ====================
    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;

    Buffer globalUBO;
    Buffer entityUBO;
    std::vector<Entity> entities;

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

    // ================
    // Core Methods
    // ================
    void Init(App* app);
    void Update(App* app);
    void Render(App* app);
    void Gui(App* app);
    void CleanUp(App* app);

    static u32 LoadTexture2D(App* app, const char* filepath);

    // ====================
    // Lighting Management
    // ====================
    void UpdateLightList();
    void CreateLights();
    void UpdateLights(App* app);

private:
    // ====================
    // Rendering Internals
    // ====================
    GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);
    void CreateEntity(App* app, u32 modelIdx, u32 textureIdx, const glm::mat4& worldMatrix);
    void RenderEntity(App* app, Entity entity, u32 entityIdx, u32 textureIdx,
        u32 textureProgramUniform, Program program);
};

#endif // _ENGINE_H_