//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "OpenGLErrorGuard.h"

#include "ImageLoader.h"
#include "ModelLoader.h"
#include "ShaderLoader.h"

#include <format>
#include "Editor.h"

App::App()
    : isRunning(true),
    deltaTime(0.0f),
    mode(Mode_Deferred_Rendering),
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
}

void App::Init()
{
    // 1. Core OpenGL state setup
    glEnable(GL_DEPTH_TEST);

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
        {orangeTexIdx, "Textures/color_orange.png"}
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
        {debugSphereIdx, "Meshes/DebugSphere.obj"}
    };

    for (auto& [idx, path] : models) 
    {
        idx = ModelLoader::LoadModel(this, path);
    }

    // 6. Camera System Initialization
    if (!worldCamera.created) 
    {
        worldCamera.SetPosition(glm::vec3(10.0f, 8.5f, 11.0f));
        worldCamera.SetTarget(glm::vec3(0.0f, 0.0f, 0.0f));
        worldCamera.SetAspectRatio(static_cast<float>(displaySize.x) / displaySize.y);
        worldCamera.SetNearFar(0.1f, 1000.0f);
        worldCamera.SetUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
        worldCamera.SetVerticalFOV(60.0f);
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
        {torusIdx, lightBlueTexIdx}
    };

    for (auto& [modelIdx, texIdx] : entities) 
    {
        CreateEntity(modelIdx, texIdx, CreateTransform(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f)));
    }

    UnmapBuffer(entityUBO);

    // 10. Framebuffer Setup
    if (!primaryFBO.Create(4, displaySize))
    {
        ELOG("[ERROR] The framebuffer was not created correctly.");
    }
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
    if (input.keys[K_6] == BUTTON_PRESS) gBufferDebugMode = 5; // Add this

    // Handle rendering mode changes
    if (needsReinit)
    {
        CleanUp();    // Release existing resources
        Init();       // Re-initialize with new mode
        needsReinit = false;
    }

    // Store previous camera state
    static glm::vec3 prevPosition = worldCamera.GetPosition();
    static glm::vec3 prevTarget = worldCamera.GetTarget();

    CameraMovement(input, worldCamera, deltaTime);

    {
        // Rotate Patrick

        static Entity* entity = &entities[0];
        static float rotationSpeed = glm::radians(45.0f); // 45 degrees per second

        float angle = rotationSpeed * deltaTime;
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

        entity->worldMatrix = entity->worldMatrix * rotation; // Apply local rotation
    }

    UpdateEntities();

    // Check if camera changed
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
        case Mode_Forward_Rendering:
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // ----------------------------------- Geometry Pass ----------------------------------- //

            Program& texturedMeshProgram = programs[forwardRenderProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            // UNIFORM BUFFER:
            // void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, globalUBO.handle, 0, globalUBO.size);

            for (auto& entity : entities)
            {
                RenderEntity(&entity, texturedMeshProgram, forwardRenderProgramUniformTexture);
            }

            glBindBufferRange(GL_UNIFORM_BUFFER, 0, 0, 0, 0); // Unbind uniform buffer (binding index 0)

            glUseProgram(0); // Unbind shader program

            // ----------------------------------- Light Debug Geometry Pass ----------------------------------- //

            // Render light spheres
            glDisable(GL_BLEND);

            if (gBufferDebugMode == 0 && enableLightDebug)
            {
                RenderLightDebugGeometry();
            }

            break;
        }
        case Mode_Deferred_Rendering:
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, displaySize.x, displaySize.y);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // ----------------------------------- Geometry Pass ----------------------------------- //

            glBindFramebuffer(GL_FRAMEBUFFER, primaryFBO.GetFramebufferHandle());

            std::vector<GLuint> drawBuffers;

            for (size_t i = 0; i < primaryFBO.GetColorAttachmentCount(); ++i)
            {
                drawBuffers.push_back(primaryFBO.GetColorAttachment(i));
            }

            glDrawBuffers(drawBuffers.size(), drawBuffers.data());

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Program& geometryProgram = programs[deferredRenderGeometryProgramIdx];
            glUseProgram(geometryProgram.handle);

            // Render all geometry to G-Buffer
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, globalUBO.handle, 0, globalUBO.size);

            for (auto& entity : entities)
            {
                RenderEntity(&entity, geometryProgram, deferredRenderProgramUniformTexture);
            }

            glUseProgram(0); // Unbind shader program

            // ----------------------------------- Depth Blit ----------------------------------- //

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, primaryFBO.GetFramebufferHandle());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(
                0, 0, displaySize.x, displaySize.y,
                0, 0, displaySize.x, displaySize.y,
                GL_DEPTH_BUFFER_BIT, GL_NEAREST
            );

            // ----------------------------------- Lighting Pass ----------------------------------- //

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            Program& quadProgram = programs[deferredRenderQuadProgramIdx];
            glUseProgram(quadProgram.handle);

            static const struct GBufferTexture
            {
                GLenum textureUnit;
                GLuint textureID;
                const char* uniformName;
            } gBufferTextures[] =
            {
                { GL_TEXTURE0, primaryFBO.GetColorAttachment(0), "uAlbedo"   },
                { GL_TEXTURE1, primaryFBO.GetColorAttachment(1), "uNormal"   },
                { GL_TEXTURE2, primaryFBO.GetColorAttachment(2), "uPosition" },
                { GL_TEXTURE3, primaryFBO.GetColorAttachment(3), "uViewDir"  },
                { GL_TEXTURE4, primaryFBO.GetDepthAttachment(),  "uDepth"    }
            };

            // Bind all textures in a loop
            for (const auto& tex : gBufferTextures)
            {
                glActiveTexture(tex.textureUnit);
                glBindTexture(GL_TEXTURE_2D, tex.textureID);
                glUniform1i(glGetUniformLocation(quadProgram.handle, tex.uniformName),
                    tex.textureUnit - GL_TEXTURE0); // Convert to 0-based index
            }

            glUniform1i(programUniformDebugMode, (GLint)gBufferDebugMode);

            // Render fullscreen quad
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            // Unbind everything after lighting pass
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0); // Unbind last active texture
            glUseProgram(0); // Unbind shader program

            // ----------------------------------- Light Debug Geometry Pass ----------------------------------- //

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);  // Prevent overwriting depth buffer
            glDisable(GL_BLEND);

            if (gBufferDebugMode == 0 && enableLightDebug)
            {
                RenderLightDebugGeometry();
            }

            glDepthMask(GL_TRUE);

            break;
        }
    }
}

void App::Gui()
{
    Editor::Draw(this);
}

void App::CleanUp()
{
    ELOG("Cleaning Up Engine");

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
    primaryFBO.Clear();

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

    primaryFBO.Clear();
    primaryFBO.Create(4, displaySize);

    worldCamera.SetAspectRatio(static_cast<float>(displaySize.x) / static_cast<float>(displaySize.y));

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

    for (int i = 0; i < gridConfig.gridSizeX; ++i) {
        for (int j = 0; j < gridConfig.gridSizeZ; ++j) {
            // Calculate position
            float x = gridConfig.minX + i * stepX;
            float z = gridConfig.minZ + j * stepZ;

            // --- Choose one color generation method below ---

            // Gradient-Based Colors
            float red = static_cast<float>(i) / (gridConfig.gridSizeX - 1);
            float green = static_cast<float>(j) / (gridConfig.gridSizeZ - 1);
            float blue = (1.0f - red);

            glm::vec3 lightColor(red, green, blue);

            gridLights.push_back({
                LightType_Point,
                lightColor,
                glm::vec3(0.0f), // Unused direction
                glm::vec3(x, gridConfig.yPos, z),
                gridConfig.gridLightConstant,   // Constant attenuation for all grid lights
                gridConfig.gridLightLinear,     // Linear attenuation for all grid lights
                gridConfig.gridLightQuadratic,  // Quadratic attenuation for all grid lights
                gridConfig.gridLightSpecularStrength              // Specular strength
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
    for (size_t i = 0; i < lights.size(); i++)
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

void App::RenderLightDebugGeometry()
{
    Program& lightSphereProgram = programs[pointLightSphereProgramIdx];
    glUseProgram(lightSphereProgram.handle);

    // Get camera matrices
    glm::mat4 view = worldCamera.ViewMatrix();
    glm::mat4 projection = worldCamera.ProjectionMatrix();

    // Bind sphere model's VAO
    Model& sphereModel = models[debugSphereIdx];
    Mesh& sphereMesh = meshes[sphereModel.meshIdx];
    Submesh& submesh = sphereMesh.submeshes[0];
    GLuint vao = FindVAO(sphereMesh, 0, lightSphereProgram);
    glBindVertexArray(vao);

    for (const auto& light : lights)
    {
        if (light.type == LightType_Point)
        {
            // Calculate model matrix
            glm::mat4 model = glm::translate(glm::mat4(1.0f), light.position);
            model = glm::scale(model, glm::vec3(0.1f)); // Adjust scale as needed
            glm::mat4 mvp = projection * view * model;

            // Set uniforms
            GLuint mvpLoc = glGetUniformLocation(lightSphereProgram.handle, "uMVP");
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

            GLuint colorLoc = glGetUniformLocation(lightSphereProgram.handle, "uColor");
            glUniform3fv(colorLoc, 1, glm::value_ptr(light.color));

            // Draw the sphere
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

    // Axis-angle rotation
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
        // Calculate new MVP
        glm::mat4 mvp = VP * entity.worldMatrix;

        // Seek to this entity's MVP offset (assuming MVP is at offset sizeof(glm::mat4))
        u32 mvpOffset = entity.entityBufferOffset + sizeof(glm::mat4);

        WriteData(entityUBO, mvpOffset, mvp);
    }

    UnmapBuffer(entityUBO);
}

GLuint App::FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    // Try finding a VAO for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i) {
        if (submesh.vaos[i].programHandle == program.handle) {
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

        // Bind texture
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

        // Draw
        Submesh& submesh = mesh.submeshes[i];
        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

        // Clean up state
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }

    // Unbind uniform buffer (binding index 1)
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, 0, 0, 0);
}
