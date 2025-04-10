#ifndef ENGINE_H
#define ENGINE_H

//
// engine.h: This file contains the types and functions relative to the engine.
//

#include "openGL_types.inl"
#include "BufferManagement.h"
#include "Camera.h"

#include <vector>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

class App
{
public:

    App();
    ~App();

    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    std::string mOpenGLInfo;

    ivec2 displaySize;

    std::vector<Texture>    textures;
    std::vector<Material>   materials;
    std::vector<Mesh>       meshes;
    std::vector<Model>      models;
    std::vector<Program>    programs;

    // program indices
    u32 renderQuadProgramIdx;
    u32 renderGeometryProgramIdx;
    u32 forwardRenderingProgramIdx;
    u32 lightSphereProgramIdx;

    // texture indices
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

    // models
    u32 patrickIdx;
    u32 planeIdx;
    u32 coneIdx;
    u32 cubeIdx;
    u32 cylinderIdx;
    u32 sphereIdx;
    u32 torusIdx;

    u32 patrickProgramUniformTexture;
    u32 fwdPatrickProgramUniformTexture;

    // Mode
    Mode mode;
    bool needsReinit;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;

    int gBufferDebugMode = 0;
    GLuint programUniformDebugMode;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    Camera worldCamera;

    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;

    Buffer globalUBO;
    Buffer entityUBO;
    std::vector<Entity> entities;

    std::vector<Light> lights;
    std::vector<Light> gridLights;
    std::vector<Light> defaultLights;
    bool gridLightsEnabled = true;
    float gridLightConstant;
    float gridLightLinear;
    float gridLightQuadratic;

    Framebuffer primaryFBO;

    std::vector<std::string> shaderErrors;
    bool showShaderErrors = false;

    static u32 LoadTexture2D(App* app, const char* filepath);

    void Init(App* app);
    void Update(App* app);
    void Render(App* app);
    void Gui(App* app);
    void CleanUp(App* app);

    void OnResize(int width, int height);

    void UpdateLightList();
    void UpdateLights(App* app);

private:

    GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);
    void CreateEntity(App* app, const u32 aModelIdx, const u32 aTextureIdx, const glm::mat4& aWorldMatrix);
    void RenderEntity(App* app, Entity entity, u32 entityIdx, u32 textureIdx, u32 textureProgramUniform, Program program);
};

#endif // ENGINE_H