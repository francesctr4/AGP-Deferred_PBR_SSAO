#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "openGL_types.inl"
#include "OpenGL_Framebuffer.h"
#include "BufferManagement.h"
#include "Camera.h"
#include "Cubemap.h"
#include <vector>

class App
{
public:

    // --- Configuration Structs ---

    struct GridLightConfig
    {
        int gridSizeX = 10;
        int gridSizeZ = 10;
        float minX = -23.0f;
        float maxX = 23.0f;
        float minZ = -23.0f;
        float maxZ = 23.0f;
        float yPos = 0.5f;
        float gridLightConstant = 1.0f;
        float gridLightLinear = 8.0f;
        float gridLightQuadratic = 3.0f;
        float gridLightSpecularStrength = 2.0f;
    };

    // --- Lifecycle Management ---

    App();
    ~App();

    void Init();
    void Update();
    void Render();
    void Gui();
    void CleanUp();

    void OnResize(int width, int height);

    // --- Lighting Management ---

    void CreateLights();
    void UpdateLightList();
    void UpdateLights();

    // --- Rendering Features ---

    void RenderGrid();

    // --- Public Members - Core ---

    bool isRunning;
    bool needsReinit;

    bool drawEditor;
    bool drawGrid;

    bool useDeferredRendering;
    bool usePBR;
    bool useSkybox;

    f32 deltaTime;
    Input input;
    Mode mode;
    ivec2 displaySize;

    // --- Public Members - Resources ---

    std::string mOpenGLInfo;

    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Model> models;
    std::vector<Program> programs;
    std::vector<Entity> entities;
    std::vector<Cubemap> cubemaps;
    u32 currentCubemapIndex;

    // --- Public Members - Rendering State ---

    Camera worldCamera;

    Framebuffer blinnPhongDeferredFBO;
    Framebuffer pbrDeferredFBO;

    GLuint embeddedVertices;
    GLuint embeddedElements;
    GLuint vao;

    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;

    Buffer globalUBO;
    Buffer entityUBO;

    // --- Public Members - Lighting ---

    std::vector<Light> lights;
    std::vector<Light> gridLights;
    std::vector<Light> defaultLights;
    bool gridLightsEnabled;
    bool enableLightDebug;
    GridLightConfig gridConfig;

    // --- Public Members - Debug/UI ---

    std::vector<std::string> shaderErrors;
    bool showShaderErrors;
    int gBufferDebugMode;

    // --- Public Members - PBR ---

    int pbrShowcase;

    float metallicInfluence;
    float roughnessInfluence;

    u32 forwardPbrIblProgramIdx;
    u32 forwardPbrDirectProgramIdx;

    u32 deferredPbrIblGeometryProgramIdx;
    u32 deferredPbrIblQuadProgramIdx;

    // --- Public Members - IBL ---

    bool enableIBL;

    u32 equirectangularToCubemapProgramIdx;
    u32 skyboxProgramIdx;
    u32 diffuseIrradianceProgramIdx;
    u32 specularPrefilterProgramIdx;
    u32 brdfIntegrationProgramIdx;

    // --- Public Members - SSAO ---

    bool enableSSAO;
    bool useAOtex;
    u32 forwardPreSSAOProgramIdx;
    u32 forwardSSAOProgramIdx;
    u32 SSAOprogramIdx;
    u32 SSAOblurProgramIdx;
    Framebuffer PreSSAOblinnPhongForwardFBO;
    Framebuffer SSAOblinnPhongForwardFBO;
    SSAOsettings ssaoSettings;

private:

    // --- Internal Helpers ---

    glm::mat4 CreateTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);
    void CreateEntity(u32 modelIdx, u32 textureIdx, const glm::mat4& worldMatrix);
    void UpdateEntities();
    GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);
    void RenderEntity(Entity* entity, Program& program, u32 programUniformTexture);
    
    void ChangeRenderMode();
    void RenderLightDebugGeometry();
    void RenderSkybox(u32 skyboxShaderIdx, u32 cubemapIdx, const glm::mat4& view, const glm::mat4& projection);
    bool LoadPBRMaterial(const std::string& directory, MaterialPBR& material);
    
    // --- SSAO Implementation ---

    void CreateResourcesSSAO();
    void DeleteResourcesSSAO();
    void CalculateSSAO(Program& shaderSSAO, GLuint gPositionID, GLuint gNormalID);
    void ApplyBlurSSAO(Program& shaderBlurSSAO);

    // --- Internal Resources ---
    
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

    // PBR Materials
    MaterialPBR cerberusMat;
    MaterialPBR lightGoldMat;
    MaterialPBR spottedRust;
    MaterialPBR fancyScaledGold;
    MaterialPBR armoredDragonScales;
    std::vector<MaterialPBR> PBRmaterials;
    u32 currentPBRmaterialIndex;

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
    u32 spherePBRIdx;

    // Shader Program Indices
    u32 deferredRenderQuadProgramIdx;
    u32 deferredRenderGeometryProgramIdx;
    u32 forwardRenderProgramIdx;
    u32 pointLightSphereProgramIdx;
    u32 gridProgramIdx;

    // Uniform Locations
    u32 deferredRenderProgramUniformTexture;
    u32 forwardRenderProgramUniformTexture;
    u32 programUniformDebugMode;

    // SSAO Resources
    GLuint ssaoFBO;
    GLuint ssaoColorBuffer;
    GLuint ssaoBlurFBO;
    GLuint ssaoColorBufferBlur;
    std::vector<glm::vec3> ssaoKernel;
    std::vector<glm::vec3> ssaoNoise;
    GLuint noiseTexture;
};

#endif // _ENGINE_H_