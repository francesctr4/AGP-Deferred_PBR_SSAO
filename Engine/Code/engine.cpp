//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"

#include "ImageLoader.h"
#include "ModelLoader.h"
#include "ShaderLoader.h"

#include "Editor.h"

#include <format>
#include <random>

App::App()
    : isRunning(true),
    deltaTime(0.0f),
    mode(Mode_PBR_Deferred_Rendering_SSAO),
    needsReinit(false),
    displaySize(0, 0),
    embeddedVertices(0),
    embeddedElements(0),
    vao(0),
    maxUniformBufferSize(0),
    uniformBlockAlignment(0),
    gridLightsEnabled(false),
    gBufferDebugMode(0),
    showShaderErrors(false),
    // Texture Indices
    diceTexIdx(0),
    whiteTexIdx(0),
    blackTexIdx(0),
    normalTexIdx(0),
    magentaTexIdx(0),
    lightBlueTexIdx(0),
    greenTexIdx(0),
    purpleTexIdx(0),
    blueTexIdx(0),
    lightGreenTexIdx(0),
    orangeTexIdx(0),
    // Model Indices
    patrickIdx(0),
    planeIdx(0),
    coneIdx(0),
    cubeIdx(0),
    cylinderIdx(0),
    sphereIdx(0),
    torusIdx(0),
    debugSphereIdx(0),
    // Shader Program Indices
    deferredRenderQuadProgramIdx(0),
    deferredRenderGeometryProgramIdx(0),
    forwardRenderProgramIdx(0),
    pointLightSphereProgramIdx(0),
    // Uniform Locations
    deferredRenderProgramUniformTexture(0),
    forwardRenderProgramUniformTexture(0),
    programUniformDebugMode(0),
    input({})
{
    worldCamera.created = false;
    currentCubemapIndex = 0;
    drawEditor = true;
    drawGrid = true;
}

void App::Init()
{
    // 1. Core OpenGL state setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // 2. Geometry Buffers Setup
        // - Vertex Buffer Object
    glGenBuffers(1, &embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

        // - Element Buffer Object
    glGenBuffers(1, &embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // - Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3UV2), (void*)offsetof(Vertex3UV2, pos));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, embeddedElements);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3UV2), (void*)offsetof(Vertex3UV2, uv));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // 3. Shader Programs Initialization
        // Forward rendering program
    forwardRenderProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/FORWARD_RENDER.glsl", "FORWARD_RENDER");

        // Deferred rendering programs
    deferredRenderGeometryProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/DEFERRED_RENDER_GEOMETRY.glsl", "DEFERRED_RENDER_GEOMETRY");

    deferredRenderQuadProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/DEFERRED_RENDER_QUAD.glsl", "DEFERRED_RENDER_QUAD");

        // Light Debug programs
    pointLightSphereProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/POINT_LIGHT_SPHERE.glsl", "POINT_LIGHT_SPHERE");
        
        // Grid program
    gridProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/GRID.glsl", "GRID");

    // ------------------------ PBR ------------------------ //
    forwardPbrIblProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/FORWARD_PBR_IBL_TEXTURED.glsl", "FORWARD_PBR_IBL_TEXTURED");

    forwardPbrDirectProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/FORWARD_PBR_DIRECT_TEXTURED.glsl", "FORWARD_PBR_DIRECT_TEXTURED");

    deferredPbrIblGeometryProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/DEFERRED_PBR_IBL_TEXTURED_GEOMETRY.glsl", "DEFERRED_PBR_IBL_TEXTURED_GEOMETRY");

    deferredPbrIblQuadProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/DEFERRED_PBR_IBL_TEXTURED_QUAD.glsl", "DEFERRED_PBR_IBL_TEXTURED_QUAD");

    equirectangularToCubemapProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/EQUIRECTANGULAR_TO_CUBEMAP.glsl", "EQUIRECTANGULAR_TO_CUBEMAP");

    skyboxProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/SKYBOX.glsl", "SKYBOX");

    diffuseIrradianceProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/DIFFUSE_IRRADIANCE_CONVOLUTION.glsl", "DIFFUSE_IRRADIANCE_CONVOLUTION");

    specularPrefilterProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/SPECULAR_PREFILTER_CONVOLUTION.glsl", "SPECULAR_PREFILTER_CONVOLUTION");

    brdfIntegrationProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/BRDF_INTEGRATION_CONVOLUTION.glsl", "BRDF_INTEGRATION_CONVOLUTION");
    // ------------------------ PBR ------------------------ //

    // ------------------------ SSAO ------------------------ //
    SSAOprogramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/SSAO.glsl", "SSAO");

    SSAOblurProgramIdx = ShaderLoader::LoadProgram(this,
        "Shaders/SSAO_BLUR.glsl", "SSAO_BLUR");

    testSSAOIdx = ShaderLoader::LoadProgram(this,
        "Shaders/DEFERRED_PBR_IBL_TEXTURED_QUAD_SSAO.glsl", "DEFERRED_PBR_IBL_TEXTURED_QUAD_SSAO");
    // ------------------------ SSAO ------------------------ //

        // Cache uniform locations
    Program& forwardRenderingProgram = programs[forwardRenderProgramIdx];
    forwardRenderProgramUniformTexture = glGetUniformLocation(forwardRenderingProgram.handle, "uAlbedo");

    Program& renderGeometryProgram = programs[deferredRenderGeometryProgramIdx];
    deferredRenderProgramUniformTexture = glGetUniformLocation(renderGeometryProgram.handle, "uAlbedo");

    Program& renderQuadProgram = programs[deferredRenderQuadProgramIdx];
    programUniformDebugMode = glGetUniformLocation(renderQuadProgram.handle, "uDebugMode");

    // 4. Texture Resources Loading
    const std::pair<GLuint&, const char*> textures[] = 
    {
        {diceTexIdx, "Textures/dice.png"},
        {whiteTexIdx, "Textures/color_white.png"},
        {blackTexIdx, "Textures/color_black.png"},
        {normalTexIdx, "Textures/color_normal.png"},
        {magentaTexIdx, "Textures/color_magenta.png"},
        {lightBlueTexIdx, "Textures/color_light_blue.png"},
        {greenTexIdx, "Textures/color_green.png"},
        {purpleTexIdx, "Textures/color_purple.png"},
        {blueTexIdx, "Textures/color_blue.png"},
        {lightGreenTexIdx, "Textures/color_light_green.png"},
        {orangeTexIdx, "Textures/color_orange.png"},

        {cerberusAlbedoIdx, "PBR/Textures/Cerberus_A.tga"},
        {cerberusMetallicIdx, "PBR/Textures/Cerberus_M.tga"},
        {cerberusNormalIdx, "PBR/Textures/Cerberus_N.tga"},
        {cerberusRoughnessIdx, "PBR/Textures/Cerberus_R.tga"},

    };

    for (auto& [idx, path] : textures) 
    {
        idx = ImageLoader::LoadTexture2D(this, path);
    }

    // 5. Model Assets Loading
    const std::pair<GLuint&, const char*> models[] = 
    {
        {patrickIdx, "Patrick/Patrick.obj"},
        {planeIdx, "Meshes/plane.obj"},
        {coneIdx, "Meshes/Cone.obj"},
        {cubeIdx, "Meshes/Cube.obj"},
        {cylinderIdx, "Meshes/Cylinder.obj"},
        {sphereIdx, "Meshes/Sphere.obj"},
        {torusIdx, "Meshes/Torus.obj"},
        {debugSphereIdx, "Meshes/DebugSphere.obj"},
        {weaponIdx, "PBR/Cerberus_LP.fbx"}
    };

    for (auto& [idx, path] : models) 
    {
        idx = ModelLoader::LoadModel(this, path);
    }

    // 6. Camera System Initialization
    if (!worldCamera.created) 
    {
        worldCamera.SetPosition(glm::vec3(75.6f, 15.7f, 30.9f));
        worldCamera.SetTarget(glm::vec3(74.8f, 15.3f, 30.4f));
        worldCamera.SetAspectRatio(static_cast<float>(displaySize.x) / displaySize.y);
        worldCamera.SetNearFar(0.1f, 1000.0f);
        worldCamera.SetUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
        worldCamera.SetVerticalFOV(90.0f);
        worldCamera.created = true;
    }

    // 7. Uniform Buffer Setup
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBlockAlignment);

    globalUBO = CreateConstantBuffer(maxUniformBufferSize);
    entityUBO = CreateConstantBuffer(maxUniformBufferSize);
    
    // 8. Lights Setup
    CreateLights();
    UpdateLights();

    // 9. Scene Entities Creation
    MapBuffer(entityUBO, GL_WRITE_ONLY);

    const std::pair<GLuint, GLuint> entities[] = 
    {
        {patrickIdx, 0},
        {planeIdx, lightGreenTexIdx},
        {coneIdx, purpleTexIdx},
        {cubeIdx, blueTexIdx},
        {cylinderIdx, orangeTexIdx},
        {sphereIdx, greenTexIdx},
        {torusIdx, lightBlueTexIdx},
        {weaponIdx, 0},
    };

    for (auto& [modelIdx, texIdx] : entities) 
    {
        CreateEntity(modelIdx, texIdx, CreateTransform(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.5f)));
    }

    UnmapBuffer(entityUBO);

    // 10. Framebuffer Setup
    if (!blinnPhongDeferredFBO.Create(4, displaySize))
    {
        ELOG("[ERROR] The framebuffer was not created correctly.");
    }

    // 11. Cubemap
    Cubemap::CreateCube();

    const std::vector<const char*> cubemapPaths =
    {
        "HDR/airport_4k.hdr",
        //"HDR/burnt_warehouse_4k.hdr",
        //"HDR/mirrored_hall_4k.hdr",
        //"HDR/cobblestone_street_night_4k.hdr",
        //"HDR/stierberg_sunrise_4k.hdr",
        //"HDR/sunset_jhbcentral_4k.hdr",
        //"HDR/table_mountain_1_4k.hdr"
    };

    // Reserve space to prevent reallocation and copying
    cubemaps.reserve(cubemapPaths.size());

    for (auto& path : cubemapPaths)
    {
        cubemaps.emplace_back(); // Add a new Cubemap to the vector
        Cubemap& newCubemap = cubemaps.back(); // Reference to the new Cubemap
        if (!newCubemap.LoadFromHDR(this, path))
        {
            cubemaps.pop_back(); // Remove if loading failed
            ELOG("Failed to load cubemap: %s", path);
        }
    }

    // 12. PBR Deferred Rendering
    if (!pbrDeferredFBO.Create(4, displaySize))
    {
        ELOG("[ERROR] The framebuffer was not created correctly.");
    }

    // 13. SSAO Resources
    CreateResourcesSSAO();
}

void App::Update()
{
#ifdef _DEBUG
    ShaderLoader::ProgramHotReload(this);
#endif

    // Debug keys
    if (input.keys[K_1] == BUTTON_PRESS) gBufferDebugMode = 0;
    if (input.keys[K_2] == BUTTON_PRESS) gBufferDebugMode = 1;
    if (input.keys[K_3] == BUTTON_PRESS) gBufferDebugMode = 2;
    if (input.keys[K_4] == BUTTON_PRESS) gBufferDebugMode = 3;
    if (input.keys[K_5] == BUTTON_PRESS) gBufferDebugMode = 4;
    if (input.keys[K_6] == BUTTON_PRESS) gBufferDebugMode = 5;
    if (input.keys[K_7] == BUTTON_PRESS) gBufferDebugMode = 6;
    if (input.keys[K_8] == BUTTON_PRESS) gBufferDebugMode = 7;

    // Cubemaps
    if (input.keys[K_M] == BUTTON_PRESS)
    {
        currentCubemapIndex = (currentCubemapIndex + 1) % cubemaps.size();
    }

    if (input.keys[K_N] == BUTTON_PRESS)
    {
        drawEditor = !drawEditor;
    }

    if (input.keys[K_B] == BUTTON_PRESS)
    {
        mode = static_cast<Mode>((static_cast<int>(mode) + 1) % ALL_MODES);
        needsReinit = true;
    }

    // Handle rendering mode changes
    if (needsReinit)
    {
        ChangeRenderMode();
        needsReinit = false;
    }

    // Store previous camera state
    static glm::vec3 prevPosition = worldCamera.GetPosition();
    static glm::vec3 prevTarget = worldCamera.GetTarget();

    CameraMovement(input, worldCamera, deltaTime);

    //{
    //    static float rotationSpeed = glm::radians(30.0f);
    //    float angle = rotationSpeed * deltaTime;
    //    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

    //    // Rotate Patrick
    //    static Entity* patrickEntity = &entities[0];
    //    patrickEntity->worldMatrix = patrickEntity->worldMatrix * rotation;

    //    // Rotate PBR Cerberus
    //    static Entity* cerberusEntity = &entities[7];
    //    cerberusEntity->worldMatrix = cerberusEntity->worldMatrix * rotation;
    //}

    UpdateEntities();

    // Check if camera changed and update lights accordingly
    if (worldCamera.GetPosition() != prevPosition ||
        worldCamera.GetTarget() != prevTarget)
    {
        UpdateLights();

        // Store new state
        prevPosition = worldCamera.GetPosition();
        prevTarget = worldCamera.GetTarget();
    }
}

void App::Render()
{
    switch (mode)
    {
        case Mode_BlinnPhong_Forward_Rendering:
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // ----------------------------------- Skybox Pass ----------------------------------- //

            if (!cubemaps.empty() && useSkybox)
            {
                RenderSkybox(skyboxProgramIdx, cubemaps[currentCubemapIndex].GetCubemapID(),
                    worldCamera.ViewMatrix(), worldCamera.ProjectionMatrix());
            }

            // ----------------------------------- Grid Pass ----------------------------------- //

            //glDepthMask(GL_FALSE);
            //// Render the grid before entities
            //RenderGrid();
            //glDepthMask(GL_TRUE);

            // ----------------------------------- Geometry Pass ----------------------------------- //

            Program& texturedMeshProgram = programs[forwardRenderProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            // UNIFORM BUFFER:
            // void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, globalUBO.handle, 0, globalUBO.size);

            for (int i = 0; i < entities.size() - 1; ++i)
            {
                RenderEntity(&entities[i], texturedMeshProgram, forwardRenderProgramUniformTexture);
            }

            glBindBufferRange(GL_UNIFORM_BUFFER, 0, 0, 0, 0);

            glUseProgram(0);

            // ----------------------------------- Light Debug Geometry Pass ----------------------------------- //

            glDisable(GL_BLEND);

            if (gBufferDebugMode == 0 && enableLightDebug)
            {
                RenderLightDebugGeometry();
            }

            break;
        }
        case Mode_BlinnPhong_Deferred_Rendering:
        {
            // ------------------------------- Geometry Pass ------------------------------- //
            glBindFramebuffer(GL_FRAMEBUFFER, blinnPhongDeferredFBO.GetFramebufferHandle());
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& geometryProgram = programs[deferredRenderGeometryProgramIdx];
            glUseProgram(geometryProgram.handle);

            glBindBufferRange(GL_UNIFORM_BUFFER, 0, globalUBO.handle, 0, globalUBO.size);

            for (int i = 0; i < entities.size() - 1; ++i)
            {
                RenderEntity(&entities[i], geometryProgram, deferredRenderProgramUniformTexture);
            }

            glUseProgram(0);

            // ------------------------------- Depth Blit ------------------------------- //
            glBindFramebuffer(GL_READ_FRAMEBUFFER, blinnPhongDeferredFBO.GetFramebufferHandle());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(
                0, 0, displaySize.x, displaySize.y,
                0, 0, displaySize.x, displaySize.y,
                GL_DEPTH_BUFFER_BIT, GL_NEAREST
            );
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ------------------------------- Lighting Pass ------------------------------- //
            // Clear only color buffer, NOT depth buffer
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& quadProgram = programs[deferredRenderQuadProgramIdx];
            glUseProgram(quadProgram.handle);

            static const struct GBufferTexture
            {
                GLenum textureUnit;
                GLuint textureID;
                const char* uniformName;
            } gBufferTextures[] =
            {
                { GL_TEXTURE0, blinnPhongDeferredFBO.GetColorAttachment(0), "uAlbedo"   },
                { GL_TEXTURE1, blinnPhongDeferredFBO.GetColorAttachment(1), "uNormal"   },
                { GL_TEXTURE2, blinnPhongDeferredFBO.GetColorAttachment(2), "uPosition" },
                { GL_TEXTURE3, blinnPhongDeferredFBO.GetColorAttachment(3), "uViewDir"  },
                { GL_TEXTURE4, blinnPhongDeferredFBO.GetDepthAttachment(),  "uDepth"    }
            };

            for (const auto& tex : gBufferTextures)
            {
                glActiveTexture(tex.textureUnit);
                glBindTexture(GL_TEXTURE_2D, tex.textureID);
                glUniform1i(glGetUniformLocation(quadProgram.handle, tex.uniformName),
                    tex.textureUnit - GL_TEXTURE0);
            }

            glUniform1i(programUniformDebugMode, (GLint)gBufferDebugMode);

            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            glBindVertexArray(0);

            glBindTexture(GL_TEXTURE_2D, 0);
            glUseProgram(0);

            // ------------------------------- Post-Lighting ------------------------------- //
            glEnable(GL_DEPTH_TEST);

            // Skybox
            if (!cubemaps.empty() && useSkybox)
            {
                glDepthFunc(GL_LEQUAL);
                RenderSkybox(skyboxProgramIdx, cubemaps[currentCubemapIndex].GetCubemapID(),
                    worldCamera.ViewMatrix(), worldCamera.ProjectionMatrix());
                glDepthFunc(GL_LESS);
            }

            // Light debug
            if (gBufferDebugMode == 0 && enableLightDebug)
            {
                RenderLightDebugGeometry();
            }

            // Cleanup
            glUseProgram(0);
            glBindTexture(GL_TEXTURE_2D, 0);

            break;
        }
        case Mode_PBR_Forward_Rendering:
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // ----------------------------------- Skybox Pass ----------------------------------- //

            if (!cubemaps.empty() && useSkybox)
            {
                RenderSkybox(skyboxProgramIdx, cubemaps[currentCubemapIndex].GetCubemapID(),
                    worldCamera.ViewMatrix(), worldCamera.ProjectionMatrix());
            }

            // ----------------------------------- Geometry Pass ----------------------------------- //
            
            //Program& forwardPbrProgram = programs[forwardPbrDirectProgramIdx];
            Program& forwardPbrProgram = programs[forwardPbrIblProgramIdx];

            glUseProgram(forwardPbrProgram.handle);

            glBindBufferRange(GL_UNIFORM_BUFFER, 0, globalUBO.handle, 0, globalUBO.size);

            Entity* entity = &entities[7];

            // Bind uniform buffer
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, entityUBO.handle, entity->entityBufferOffset, entity->entityBufferSize);

            Model& model = models[entity->modelIdx];
            Mesh& mesh = meshes[model.meshIdx];

            // Bind pre-computed IBL data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemaps[currentCubemapIndex].GetDiffuseIrradianceMap());
            glUniform1i(glGetUniformLocation(forwardPbrProgram.handle, "irradianceMap"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemaps[currentCubemapIndex].GetSpecularPrefilterMap());
            glUniform1i(glGetUniformLocation(forwardPbrProgram.handle, "prefilterMap"), 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, cubemaps[currentCubemapIndex].GetBRFDlookUpTexture());
            glUniform1i(glGetUniformLocation(forwardPbrProgram.handle, "brdfLUT"), 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusAlbedoIdx].handle);
            glUniform1i(glGetUniformLocation(forwardPbrProgram.handle, "albedoMap"), 3);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusNormalIdx].handle);
            glUniform1i(glGetUniformLocation(forwardPbrProgram.handle, "normalMap"), 4);

            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusMetallicIdx].handle);
            glUniform1i(glGetUniformLocation(forwardPbrProgram.handle, "metallicMap"), 5);

            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusRoughnessIdx].handle);
            glUniform1i(glGetUniformLocation(forwardPbrProgram.handle, "roughnessMap"), 6);

            for (u32 i = 0; i < mesh.submeshes.size(); ++i)
            {
                GLuint vao = FindVAO(mesh, i, forwardPbrProgram);
                glBindVertexArray(vao);

                Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

                glBindVertexArray(0);
            }

            glBindTexture(GL_TEXTURE_2D, 0);

            // Unbind uniform buffer (binding index 1)
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, 0, 0, 0);

            glBindBufferRange(GL_UNIFORM_BUFFER, 0, 0, 0, 0);

            glUseProgram(0);

            // ----------------------------------- Light Debug Geometry Pass ----------------------------------- //

            glDisable(GL_BLEND);

            if (gBufferDebugMode == 0 && enableLightDebug)
            {
                RenderLightDebugGeometry();
            }

            break;
        }
        case Mode_PBR_Deferred_Rendering:
        {
            // ------------------------------- Geometry Pass ------------------------------- //
            glBindFramebuffer(GL_FRAMEBUFFER, pbrDeferredFBO.GetFramebufferHandle());
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& geometryProgram = programs[deferredPbrIblGeometryProgramIdx];
            glUseProgram(geometryProgram.handle);
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, globalUBO.handle, 0, globalUBO.size);

            Entity& entity = entities[7];

            // Bind entity's uniform buffer
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, entityUBO.handle,
                entity.entityBufferOffset, entity.entityBufferSize);

            Model& model = models[entity.modelIdx];
            Mesh& mesh = meshes[model.meshIdx];

            // Bind PBR textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusAlbedoIdx].handle);
            glUniform1i(glGetUniformLocation(geometryProgram.handle, "uAlbedo"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusNormalIdx].handle);
            glUniform1i(glGetUniformLocation(geometryProgram.handle, "uNormal"), 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusMetallicIdx].handle);
            glUniform1i(glGetUniformLocation(geometryProgram.handle, "uMetallic"), 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusRoughnessIdx].handle);
            glUniform1i(glGetUniformLocation(geometryProgram.handle, "uRoughness"), 3);

            // Draw submeshes
            for (u32 i = 0; i < mesh.submeshes.size(); ++i)
            {
                GLuint vao = FindVAO(mesh, i, geometryProgram);
                glBindVertexArray(vao);
                Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(),
                    GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                glBindVertexArray(0);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ------------------------------- Depth Blit ------------------------------- //
            glBindFramebuffer(GL_READ_FRAMEBUFFER, pbrDeferredFBO.GetFramebufferHandle());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(
                0, 0, displaySize.x, displaySize.y,
                0, 0, displaySize.x, displaySize.y,
                GL_DEPTH_BUFFER_BIT, GL_NEAREST
            );
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ------------------------------- Lighting Pass ------------------------------- //
            // Clear only color buffer, NOT depth buffer
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& quadProgram = programs[deferredPbrIblQuadProgramIdx];
            glUseProgram(quadProgram.handle);

            // Bind G-buffer textures
            struct GBufferBinding {
                GLenum unit;
                GLuint texture;
                const char* name;
            } gBufferBindings[] = {
                {GL_TEXTURE0, pbrDeferredFBO.GetColorAttachment(0), "gAlbedoRoughness"},
                {GL_TEXTURE1, pbrDeferredFBO.GetColorAttachment(1), "gNormalMetallic"},
                {GL_TEXTURE2, pbrDeferredFBO.GetColorAttachment(2), "gPosition"},
                {GL_TEXTURE3, pbrDeferredFBO.GetColorAttachment(3), "gViewDir"},
                {GL_TEXTURE4, pbrDeferredFBO.GetDepthAttachment(),  "gDepth"}
            };

            for (auto& binding : gBufferBindings) {
                glActiveTexture(binding.unit);
                glBindTexture(GL_TEXTURE_2D, binding.texture);
                glUniform1i(glGetUniformLocation(quadProgram.handle, binding.name),
                    binding.unit - GL_TEXTURE0);
            }

            // Bind IBL textures
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemaps[currentCubemapIndex].GetDiffuseIrradianceMap());
            glUniform1i(glGetUniformLocation(quadProgram.handle, "irradianceMap"), 4);

            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemaps[currentCubemapIndex].GetSpecularPrefilterMap());
            glUniform1i(glGetUniformLocation(quadProgram.handle, "prefilterMap"), 5);

            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, cubemaps[currentCubemapIndex].GetBRFDlookUpTexture());
            glUniform1i(glGetUniformLocation(quadProgram.handle, "brdfLUT"), 6);

            glUniform1i(glGetUniformLocation(quadProgram.handle, "gDebugMode"), (GLint)gBufferDebugMode);

            // Render fullscreen quad
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            glBindVertexArray(0);

            // ------------------------------- Post-Lighting ------------------------------- //
            glEnable(GL_DEPTH_TEST);

            // Skybox
            if (!cubemaps.empty() && useSkybox) 
            {
                glDepthFunc(GL_LEQUAL);
                RenderSkybox(skyboxProgramIdx, cubemaps[currentCubemapIndex].GetCubemapID(),
                    worldCamera.ViewMatrix(), worldCamera.ProjectionMatrix());
                glDepthFunc(GL_LESS);
            }

            // Light debug
            if (gBufferDebugMode == 0 && enableLightDebug) 
            {
                RenderLightDebugGeometry();
            }

            // Cleanup
            glUseProgram(0);
            glBindTexture(GL_TEXTURE_2D, 0);

            break;
        }
        case Mode_PBR_Deferred_Rendering_SSAO:
        {
            // ------------------------------- Geometry Pass -------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, pbrDeferredFBO.GetFramebufferHandle());
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& geometryProgram = programs[deferredPbrIblGeometryProgramIdx];
            glUseProgram(geometryProgram.handle);
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, globalUBO.handle, 0, globalUBO.size);

            Entity& entity = entities[7];
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, entityUBO.handle, entity.entityBufferOffset, entity.entityBufferSize);

            Model& model = models[entity.modelIdx];
            Mesh& mesh = meshes[model.meshIdx];

            // Bind PBR textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusAlbedoIdx].handle);
            glUniform1i(glGetUniformLocation(geometryProgram.handle, "uAlbedo"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusNormalIdx].handle);
            glUniform1i(glGetUniformLocation(geometryProgram.handle, "uNormal"), 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusMetallicIdx].handle);
            glUniform1i(glGetUniformLocation(geometryProgram.handle, "uMetallic"), 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, textures[cerberusRoughnessIdx].handle);
            glUniform1i(glGetUniformLocation(geometryProgram.handle, "uRoughness"), 3);

            // Draw geometry
            for (u32 i = 0; i < mesh.submeshes.size(); ++i) {
                GLuint vao = FindVAO(mesh, i, geometryProgram);
                glBindVertexArray(vao);
                Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                glBindVertexArray(0);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ------------------------------- Depth Blit -------------------------------
            glBindFramebuffer(GL_READ_FRAMEBUFFER, pbrDeferredFBO.GetFramebufferHandle());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, displaySize.x, displaySize.y, 0, 0, displaySize.x, displaySize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ------------------------------- SSAO Pass -------------------------------
            CalculateSSAO(programs[SSAOprogramIdx], pbrDeferredFBO.GetColorAttachment(2), pbrDeferredFBO.GetColorAttachment(1));
            ApplyBlurSSAO(programs[SSAOblurProgramIdx]);

            // ------------------------------- Lighting Pass -------------------------------
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glViewport(0, 0, displaySize.x, displaySize.y);

            Program& quadProgram = programs[testSSAOIdx];
            glUseProgram(quadProgram.handle);

            // Re-bind G-buffer textures after SSAO passes
            struct GBufferBinding {
                GLenum unit;
                GLuint texture;
                const char* name;
            } gBufferBindings[] = {
                {GL_TEXTURE0, pbrDeferredFBO.GetColorAttachment(0), "gAlbedoRoughness"},
                {GL_TEXTURE1, pbrDeferredFBO.GetColorAttachment(1), "gNormalMetallic"},
                {GL_TEXTURE2, pbrDeferredFBO.GetColorAttachment(2), "gPosition"},
                {GL_TEXTURE3, pbrDeferredFBO.GetColorAttachment(3), "gViewDir"},
                {GL_TEXTURE4, pbrDeferredFBO.GetDepthAttachment(),  "gDepth"}
            };

            for (auto& binding : gBufferBindings) {
                glActiveTexture(binding.unit);
                glBindTexture(GL_TEXTURE_2D, binding.texture);
                glUniform1i(glGetUniformLocation(quadProgram.handle, binding.name), binding.unit - GL_TEXTURE0);
            }

            // Bind SSAO texture
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
            glUniform1i(glGetUniformLocation(quadProgram.handle, "aoMap"), 5);

            // Bind IBL textures
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemaps[currentCubemapIndex].GetDiffuseIrradianceMap());
            glUniform1i(glGetUniformLocation(quadProgram.handle, "irradianceMap"), 6);

            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemaps[currentCubemapIndex].GetSpecularPrefilterMap());
            glUniform1i(glGetUniformLocation(quadProgram.handle, "prefilterMap"), 7);

            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D, cubemaps[currentCubemapIndex].GetBRFDlookUpTexture());
            glUniform1i(glGetUniformLocation(quadProgram.handle, "brdfLUT"), 8);

            glUniform1i(glGetUniformLocation(quadProgram.handle, "gDebugMode"), (GLint)gBufferDebugMode);

            // Render fullscreen quad
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            glBindVertexArray(0);

            // ------------------------------- Post-Lighting -------------------------------
            glEnable(GL_DEPTH_TEST);

            // Skybox
            if (!cubemaps.empty() && useSkybox) {
                glDepthFunc(GL_LEQUAL);
                RenderSkybox(skyboxProgramIdx, cubemaps[currentCubemapIndex].GetCubemapID(),
                    worldCamera.ViewMatrix(), worldCamera.ProjectionMatrix());
                glDepthFunc(GL_LESS);
            }

            // Light debug
            if (gBufferDebugMode == 0 && enableLightDebug) {
                RenderLightDebugGeometry();
            }

            // Cleanup
            glUseProgram(0);
            glBindTexture(GL_TEXTURE_2D, 0);
            break;
        }
    }
}

void App::Gui()
{
    if (drawEditor) 
    {
        Editor::Draw(this);
    }
}

void App::CleanUp()
{
    ELOG("Cleaning Up Engine");

    // --- Clean Up Cubemaps ---
    cubemaps.clear();
    Cubemap::ReleaseCube();

    // --- Delete Textures ---
    for (auto& texture : textures)
    {
        glDeleteTextures(1, &texture.handle);
    }
    textures.clear();

    // --- Delete Shader Programs ---
    for (auto& program : programs)
    {
        glDeleteProgram(program.handle);
    }
    programs.clear();

    // --- Delete Main VAO ---
    if (vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    // --- Delete Element/Index Buffers ---
    if (embeddedElements != 0)
    {
        glDeleteBuffers(1, &embeddedElements);
        embeddedElements = 0;
    }

    // --- Delete Vertex Buffers ---
    if (embeddedVertices != 0)
    {
        glDeleteBuffers(1, &embeddedVertices);
        embeddedVertices = 0;
    }

    // --- Clean Up Meshes (VAOs, VBOs, and IBOs) ---
    for (auto& mesh : meshes)
    {
        // Delete VAOs for each submesh
        for (auto& submesh : mesh.submeshes)
        {
            for (auto& vao : submesh.vaos)
            {
                glDeleteVertexArrays(1, &vao.handle);
            }
            submesh.vaos.clear();
        }
        glDeleteBuffers(1, &mesh.vertexBufferHandle);
        glDeleteBuffers(1, &mesh.indexBufferHandle);
    }
    meshes.clear();

    // --- Clean Up Models and Materials ---
    models.clear();
    materials.clear();

    // --- Clean Up Uniform Buffers ---
    if (globalUBO.handle != 0)
    {
        glDeleteBuffers(1, &globalUBO.handle);
        globalUBO.handle = 0;
    }
    if (entityUBO.handle != 0)
    {
        glDeleteBuffers(1, &entityUBO.handle);
        entityUBO.handle = 0;
    }

    // --- Clean Up Entities ---
    entities.clear();

    // --- Clean Up FBO ---
    blinnPhongDeferredFBO.Clear();

    lights.clear();
    gridLights.clear();
    defaultLights.clear();
}

App::~App()
{

}

void App::OnResize(int width, int height)
{
    if (width == 0 && height == 0) return;

    displaySize = vec2(width, height);
    worldCamera.SetAspectRatio(static_cast<float>(displaySize.x) / static_cast<float>(displaySize.y));

    blinnPhongDeferredFBO.Clear();
    blinnPhongDeferredFBO.Create(4, displaySize);

    pbrDeferredFBO.Clear();
    pbrDeferredFBO.Create(4, displaySize);

    DeleteResourcesSSAO();
    CreateResourcesSSAO();

    UpdateEntities();
}

// ------------------------------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------------------------------ //

void App::CreateLights()
{
    // Clear existing lights to prevent duplication
    gridLights.clear();

    float stepX = (gridConfig.maxX - gridConfig.minX) / (gridConfig.gridSizeX - 1);
    float stepZ = (gridConfig.maxZ - gridConfig.minZ) / (gridConfig.gridSizeZ - 1);

    for (int i = 0; i < gridConfig.gridSizeX; ++i) 
    {
        for (int j = 0; j < gridConfig.gridSizeZ; ++j) 
        {
            float x = gridConfig.minX + i * stepX;
            float z = gridConfig.minZ + j * stepZ;

            float red = static_cast<float>(i) / (gridConfig.gridSizeX - 1);
            float green = static_cast<float>(j) / (gridConfig.gridSizeZ - 1);
            float blue = (1.0f - red);

            glm::vec3 lightColor(red, green, blue);

            gridLights.push_back({
                LightType_Point,
                lightColor,
                glm::vec3(0.0f), // Unused direction
                glm::vec3(x, gridConfig.yPos, z),
                gridConfig.gridLightConstant,
                gridConfig.gridLightLinear,
                gridConfig.gridLightQuadratic,
                gridConfig.gridLightSpecularStrength
                });
        }
    }

    if (defaultLights.empty())
    {
        defaultLights.push_back({
            LightType_Directional,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, -1.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            1.0f, 0.09f, 0.032f, 0.5f
            });

        defaultLights.push_back({
            LightType_Point,
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 5.8f, 6.2f),
            0.78f, 0.1f, 0.010f, 0.010f
            });
    }

    UpdateLightList();
}

void App::UpdateLightList()
{
    lights.clear();

    if (gridLightsEnabled) 
    {
        lights.insert(lights.end(), gridLights.begin(), gridLights.end());
    }
    else 
    {
        lights.insert(lights.end(), defaultLights.begin(), defaultLights.end());
    }
}

void App::UpdateLights()
{
    MapBuffer(globalUBO, GL_WRITE_ONLY);

    PushVec3(globalUBO, worldCamera.GetPosition());
    PushUInt(globalUBO, lights.size());

    for (size_t i = 0; i < lights.size(); ++i)
    {
        AlignHead(globalUBO, sizeof(vec4));

        Light& light = lights[i];

        PushUInt(globalUBO, static_cast<unsigned int>(light.type));
        PushVec3(globalUBO, light.color);
        PushVec3(globalUBO, light.direction);
        PushVec3(globalUBO, light.position);
        PushFloat(globalUBO, light.constant);
        PushFloat(globalUBO, light.linear);
        PushFloat(globalUBO, light.quadratic);
        PushFloat(globalUBO, light.specularStrength);
    }

    UnmapBuffer(globalUBO);
}

void App::RenderGrid()
{
    if (drawGrid == false) return;

    //// Bind the default framebuffer (or adjust if using a specific FBO)
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Program& gridProgram = programs[gridProgramIdx];
    glUseProgram(gridProgram.handle);

    // Calculate camera frustum parameters
    float znear = worldCamera.GetZNear();
    float aspectRatio = worldCamera.GetAspectRatio();
    float fovY = worldCamera.GetVerticalFOV();
    float fovRadians = glm::radians(fovY);
    float top = znear * tan(fovRadians / 2.0f);
    float bottom = -top;
    float right = top * aspectRatio;
    float left = -right;

    // Get camera matrices
    glm::mat4 viewMatrix = worldCamera.ViewMatrix();
    glm::mat4 projectionMatrix = worldCamera.ProjectionMatrix();
    glm::mat4 worldMatrix = glm::inverse(viewMatrix);

    // Set uniforms
    glUniform1f(glGetUniformLocation(gridProgram.handle, "left"), left);
    glUniform1f(glGetUniformLocation(gridProgram.handle, "right"), right);
    glUniform1f(glGetUniformLocation(gridProgram.handle, "bottom"), bottom);
    glUniform1f(glGetUniformLocation(gridProgram.handle, "top"), top);
    glUniform1f(glGetUniformLocation(gridProgram.handle, "znear"), znear);
    glUniformMatrix4fv(glGetUniformLocation(gridProgram.handle, "worldMatrix"), 1, GL_FALSE, glm::value_ptr(worldMatrix));
    glUniformMatrix4fv(glGetUniformLocation(gridProgram.handle, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(gridProgram.handle, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Render full-screen quad using the embedded VAO
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

void App::RenderLightDebugGeometry()
{
    Program& lightSphereProgram = programs[pointLightSphereProgramIdx];
    glUseProgram(lightSphereProgram.handle);

    glm::mat4 view = worldCamera.ViewMatrix();
    glm::mat4 projection = worldCamera.ProjectionMatrix();

    Model& sphereModel = models[debugSphereIdx];
    Mesh& sphereMesh = meshes[sphereModel.meshIdx];
    Submesh& submesh = sphereMesh.submeshes[0];
    GLuint vao = FindVAO(sphereMesh, 0, lightSphereProgram);

    glBindVertexArray(vao);

    for (const auto& light : lights)
    {
        if (light.type == LightType_Point)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), light.position);
            model = glm::scale(model, glm::vec3(0.1f));
            glm::mat4 mvp = projection * view * model;

            GLuint mvpLoc = glGetUniformLocation(lightSphereProgram.handle, "uMVP");
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

            GLuint colorLoc = glGetUniformLocation(lightSphereProgram.handle, "uColor");
            glUniform3fv(colorLoc, 1, glm::value_ptr(light.color));

            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

// ---------------------------------------------------------------------------------------------------------- //
// ---------------------------------------------------------------------------------------------------------- //
// ---------------------------------------------------------------------------------------------------------- //

glm::mat4 App::CreateTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
{
    glm::mat4 transform = glm::identity<glm::mat4>();

    transform = glm::translate(transform, translation);

    float angleDegrees = glm::length(rotation);

    if (angleDegrees > 0.0001f)
    {
        glm::vec3 axis = glm::normalize(rotation);
        float angleRadians = glm::radians(angleDegrees);
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angleRadians, axis);
        transform *= rot;
    }

    transform = glm::scale(transform, scale);

    return transform;
}

void App::CreateEntity(const u32 aModelIdx, const u32 aTextureIdx, const glm::mat4& aWorldMatrix)
{
    Entity entity{};
    AlignHead(entityUBO, uniformBlockAlignment);
    entity.entityBufferOffset = entityUBO.head;

    entity.worldMatrix = aWorldMatrix;
    entity.modelIdx = aModelIdx;
    entity.textureIdx = aTextureIdx;

    // Only push world matrix during creation
    PushMat4(entityUBO, entity.worldMatrix);
    // Reserve space for MVP (will be updated later)
    PushMat4(entityUBO, glm::identity<glm::mat4>());

    entity.entityBufferSize = entityUBO.head - entity.entityBufferOffset;
    entities.push_back(entity);
}

void App::UpdateEntities()
{
    MapBuffer(entityUBO, GL_WRITE_ONLY);

    glm::mat4 VP = worldCamera.ProjectionMatrix() * worldCamera.ViewMatrix();

    for (auto& entity : entities)
    {
        glm::mat4 mvp = VP * entity.worldMatrix;

        AlignHead(entityUBO, uniformBlockAlignment);
        PushMat4(entityUBO, entity.worldMatrix);
        PushMat4(entityUBO, mvp);
    }

    UnmapBuffer(entityUBO);
}

GLuint App::FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    // Try finding a VAO for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i) 
    {
        if (submesh.vaos[i].programHandle == program.handle) 
        {
            return submesh.vaos[i].handle;
        }
    }

    GLuint vaoHandle = 0;
    // Create a new VAO for this submesh/program
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        // We have to link all vertex input attributes to attributes in the vertex buffer
        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            bool attributeWasLinked = false;

            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset; // attribute offset + vertex offset
                    const u32 stride = submesh.vertexBufferLayout.stride;
                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(uintptr_t)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }

            assert(attributeWasLinked); // The submesh should provide an attribute for each vertex input
        }

        glBindVertexArray(0);
    }

    // Store it in the list of VAOs for this submesh
    VAO vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

void App::RenderEntity(Entity* entity, Program& program, u32 programUniformTexture)
{
    // Bind uniform buffer
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, entityUBO.handle, entity->entityBufferOffset, entity->entityBufferSize);

    Model& model = models[entity->modelIdx];
    Mesh& mesh = meshes[model.meshIdx];

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        GLuint vao = FindVAO(mesh, i, program);
        glBindVertexArray(vao);

        u32 submeshMaterialIdx = model.materialIdx[i];
        Material& submeshMaterial = materials[submeshMaterialIdx];

        glActiveTexture(GL_TEXTURE0);
        if (submeshMaterial.albedoTextureIdx > 0)
        {
            glBindTexture(GL_TEXTURE_2D, textures[submeshMaterial.albedoTextureIdx].handle);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, textures[entity->textureIdx].handle);
        }
        glUniform1i(programUniformTexture, 0);

        Submesh& submesh = mesh.submeshes[i];
        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }

    // Unbind uniform buffer (binding index 1)
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, 0, 0, 0);
}

void App::RenderSkybox(u32 skyboxShaderIdx, u32 cubemapIdx, const glm::mat4& view, const glm::mat4& projection)
{
    glDepthMask(GL_FALSE);
    GLint prevDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc); // Save current depth function
    glDepthFunc(GL_LEQUAL); // Change depth function to allow depth == 1.0

    Program& skyboxProgram = programs[skyboxShaderIdx];
    glUseProgram(skyboxProgram.handle);

    // Set up matrices (remove translation from view matrix)
    glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(view));
    GLint viewLoc = glGetUniformLocation(skyboxProgram.handle, "view");
    GLint projectionLoc = glGetUniformLocation(skyboxProgram.handle, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewWithoutTranslation));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Bind cubemap and render
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapIdx);

    Cubemap::RenderCube();

    glDepthFunc(prevDepthFunc); // Restore original depth function
    glDepthMask(GL_TRUE);
}

void App::ChangeRenderMode()
{
    // Handle reinit of features that change between forward and deferred rendering

}

float Lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void App::CreateResourcesSSAO()
{
    // also create framebuffer to hold SSAO processing stage 
    // -----------------------------------------------------
    glGenFramebuffers(1, &ssaoFBO);  
    glGenFramebuffers(1, &ssaoBlurFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, displaySize.x, displaySize.y, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, displaySize.x, displaySize.y, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // generate sample kernel
    // ----------------------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // generate noise texture
    // ----------------------
    
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void App::DeleteResourcesSSAO()
{
    // Delete framebuffers
    if (ssaoFBO) {
        glDeleteFramebuffers(1, &ssaoFBO);
        ssaoFBO = 0;
    }
    if (ssaoBlurFBO) {
        glDeleteFramebuffers(1, &ssaoBlurFBO);
        ssaoBlurFBO = 0;
    }

    // Delete textures
    if (ssaoColorBuffer) {
        glDeleteTextures(1, &ssaoColorBuffer);
        ssaoColorBuffer = 0;
    }
    if (ssaoColorBufferBlur) {
        glDeleteTextures(1, &ssaoColorBufferBlur);
        ssaoColorBufferBlur = 0;
    }
    if (noiseTexture) {
        glDeleteTextures(1, &noiseTexture);
        noiseTexture = 0;
    }

    // Clear vectors (though they'll be repopulated)
    ssaoKernel.clear();
    ssaoNoise.clear();
}

void App::CalculateSSAO(Program& shaderSSAO, GLuint gPositionID, GLuint gNormalID)
{
    // 2. generate SSAO texture
    // -----------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    //glViewport(0, 0, displaySize.x, displaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(shaderSSAO.handle, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(shaderSSAO.handle, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderSSAO.handle, "texNoise"), 2);

    glUniform1i(glGetUniformLocation(shaderSSAO.handle, "kernelSize"), 64);
    glUniform1f(glGetUniformLocation(shaderSSAO.handle, "radius"), 0.5);
    glUniform1f(glGetUniformLocation(shaderSSAO.handle, "bias"), 0.025);

    glUniform1f(glGetUniformLocation(shaderSSAO.handle, "SCREEN_WIDTH"), displaySize.x);
    glUniform1f(glGetUniformLocation(shaderSSAO.handle, "SCREEN_HEIGHT"), displaySize.y);

    glUseProgram(shaderSSAO.handle);

    // Send kernel + rotation 
    for (unsigned int i = 0; i < 64; ++i)
    {
        GLint location = glGetUniformLocation(shaderSSAO.handle, ("samples[" + std::to_string(i) + "]").c_str());
        glUniform3fv(location, 1, &ssaoKernel[i][0]);
    }

    glm::mat4 view = worldCamera.ViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderSSAO.handle, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 projection = worldCamera.ProjectionMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderSSAO.handle, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPositionID);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormalID);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    Cubemap::RenderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void App::ApplyBlurSSAO(Program& shaderBlurSSAO)
{
    // 3. Blur SSAO Texture to Remove Noise
    // ------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    //glViewport(0, 0, displaySize.x, displaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(shaderBlurSSAO.handle, "ssaoInput"), 0);

    glUseProgram(shaderBlurSSAO.handle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

    Cubemap::RenderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
