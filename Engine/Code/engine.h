#ifndef _ENGINE_H_
#define _ENGINE_H_

//
// engine.h: Engine types and core functionality
//

#include "openGL_types.inl"
#include "OpenGL_Framebuffer.h"
#include "BufferManagement.h"
#include "Camera.h"
#include "Cubemap.h"
#include <vector>

struct GridLightConfig
{
    int gridSizeX = 28;  // Number of columns (X-axis)
    int gridSizeZ = 28;  // Number of rows (Z-axis)
    float minX = -23.0f; // Start X range
    float maxX = 23.0f;  // End X range
    float minZ = -23.0f; // Start Z range
    float maxZ = 23.0f;  // End Z range
    float yPos = 0.1f;   // Fixed Y position

    float gridLightConstant = 1.0f;
    float gridLightLinear = 8.0f;
    float gridLightQuadratic = 3.0f;
    float gridLightSpecularStrength = 2.0f;
};

class App
{
public:

    App();
    ~App();

    void Init();
    void Update();
    void Render();
    void Gui();
    void CleanUp();

    void OnResize(int width, int height);

    // ------------------------------------------------------------- //

    void CreateLights();
    void UpdateLightList();
    void UpdateLights();

private:

    void RenderLightDebugGeometry();

    glm::mat4 CreateTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);
    void CreateEntity(u32 modelIdx, u32 textureIdx, const glm::mat4& worldMatrix);
    void UpdateEntities();
    GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);
    void RenderEntity(Entity* entity, Program& program, u32 programUniformTexture);

    void ChangeRenderMode();

public:

    // Initialization
    Cubemap* environmentMap;

    // Core
    bool isRunning;
    f32 deltaTime;
    Input input;
    Mode mode;
    bool needsReinit;
    ivec2 displaySize;

    // Graphics Resources
    std::string mOpenGLInfo;

    // Resource pools
    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh>     meshes;
    std::vector<Model>    models;
    std::vector<Program>  programs;
    std::vector<Entity> entities;

    // Rendering State
    Camera worldCamera;
    Framebuffer primaryFBO;

    // Embedded geometry
    GLuint embeddedVertices;
    GLuint embeddedElements;
    GLuint vao;

    // Buffers and UBOs
    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;
    Buffer globalUBO;
    Buffer entityUBO;

    // Lighting
    std::vector<Light> lights;
    std::vector<Light> gridLights;
    std::vector<Light> defaultLights;

    bool gridLightsEnabled = true;
    bool enableLightDebug = false;

    GridLightConfig gridConfig;

    // Debug/UI State
    std::vector<std::string> shaderErrors;
    bool showShaderErrors = false;

    int gBufferDebugMode = 0;

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
    u32 debugSphereIdx;
    u32 weaponIdx;

    // Shader Program Indices
    u32 deferredRenderQuadProgramIdx;
    u32 deferredRenderGeometryProgramIdx;
    u32 forwardRenderProgramIdx;
    u32 pointLightSphereProgramIdx;

    u32 pbrProgramIdx;
    u32 skyboxProgramIdx;
    u32 equirectangularProgramIdx;

    // Uniform locations
    u32 deferredRenderProgramUniformTexture;
    u32 forwardRenderProgramUniformTexture;
    u32 programUniformDebugMode;

};

#endif // _ENGINE_H_