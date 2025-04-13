//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "OpenGLErrorGuard.h"
#include "ModelLoader.h"

#include <stb_image.h>
#include <stb_image_write.h>
#include <format>
#include "Editor.h"

void App::UpdateLightList()
{
    lights.clear();
    if (gridLightsEnabled)
        lights.insert(lights.end(), gridLights.begin(), gridLights.end());
    lights.insert(lights.end(), defaultLights.begin(), defaultLights.end());
}

void App::CreateLights() 
{
    // Clear existing lights to prevent duplication
    gridLights.clear();
    defaultLights.clear();

    int gridSizeX = 40;  // Number of columns (X-axis)
    int gridSizeZ = 40;  // Number of rows (Z-axis)
    float minX = -15.0f; // Start X range
    float maxX = 15.0f;  // End X range
    float minZ = -15.0f; // Start Z range
    float maxZ = 15.0f;  // End Z range
    float yPos = 5.0f;   // Fixed Y position

    // In App::App() constructor, before the grid light loop:
    gridLightConstant = 1.0f;
    gridLightLinear = 0.5f;
    gridLightQuadratic = 0.5f;

    float stepX = (maxX - minX) / (gridSizeX - 1);
    float stepZ = (maxZ - minZ) / (gridSizeZ - 1);

    for (int i = 0; i < gridSizeX; ++i) {
        for (int j = 0; j < gridSizeZ; ++j) {
            // Calculate position
            float x = minX + i * stepX;
            float z = minZ + j * stepZ;

            // --- Choose one color generation method below ---

            // Gradient-Based Colors
            float red = static_cast<float>(i) / (gridSizeX - 1) * 0.1f;
            float green = static_cast<float>(j) / (gridSizeZ - 1) * 0.1f;
            float blue = (1.0f - red) * 0.1f;

            // Sinusoidal Variation
            // float red = (sin(i * 0.5f) + 1.0f) * 0.5f;
            // float green = (cos(j * 0.5f) + 1.0f) * 0.5f;
            // float blue = (sin((i + j) * 0.3f) + 1.0f) * 0.5f;

            glm::vec3 lightColor(red, green, blue);

            gridLights.push_back({
                LightType_Point,
                lightColor,
                glm::vec3(0.0f), // Unused direction
                glm::vec3(x, yPos, z),
                gridLightConstant,   // Constant attenuation for all grid lights
                gridLightLinear,     // Linear attenuation for all grid lights
                gridLightQuadratic,  // Quadratic attenuation for all grid lights
                0.010f               // Specular strength
                });
        }
    }

    defaultLights.push_back({
        LightType_Directional,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f), // Direction (points downward)
        glm::vec3(0.0f, 0.0f, 0.0f),   // Unused position
        1.0f,   // unused for directional
        0.09f,  // unused
        0.032f, // unused
        0.5f    // specular strength
        });

    defaultLights.push_back({
                LightType_Point,
                glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f), // Unused direction
                glm::vec3(0.0f, 10.0f, 0.0f),
                1.0f,   // constant attenuation
                0.1f,  // linear attenuation
                0.010f, // quadratic attenuation
                0.010f    // specular strength// Position
        });

    // Combine into lights based on flag
    gridLightsEnabled = false;

    UpdateLightList();
}

App::App()
{
    this->mode = Mode_Deferred_Rendering;
}

App::~App()
{

}

void UpdateEntityUBO(App* app)
{
    MapBuffer(app->entityUBO, GL_WRITE_ONLY);

    glm::mat4 VP = app->worldCamera.ProjectionMatrix() * app->worldCamera.ViewMatrix();

    for (auto& entity : app->entities)
    {
        // Calculate new MVP
        glm::mat4 mvp = VP * entity.worldMatrix;

        // Seek to this entity's MVP offset (assuming MVP is at offset sizeof(glm::mat4))
        u32 mvpOffset = entity.entityBufferOffset + sizeof(glm::mat4);

        WriteData(app->entityUBO, mvpOffset, mvp);
    }

    UnmapBuffer(app->entityUBO);
}

GLuint CreateProgramFromSource(App* app, String programSource, const char* shaderName)
{
    app->shaderErrors.clear();

    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);

        app->shaderErrors.push_back(std::format(
            "glCompileShader() failed with vertex shader '{}'\nReported message:\n{}\n",
            shaderName, infoLogBuffer));

        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);

        app->shaderErrors.push_back(std::format(
            "glCompileShader() failed with fragment shader '{}'\nReported message:\n{}\n",
            shaderName, infoLogBuffer));

        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);

        app->shaderErrors.push_back(std::format(
            "glLinkProgram() failed with program '{}'\nReported message:\n{}\n",
            shaderName, infoLogBuffer));

        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(app, programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

    if (program.handle != 0) 
    {
        GLint attributeCount = 0UL;
        glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

        for (size_t i = 0; i < attributeCount; ++i) 
        {
            GLchar attributeName[248];
            GLsizei attributeNameLength = 0UL;
            GLsizei attributeSize = 0UL;
            GLenum attributeType = 0UL;

            glGetActiveAttrib(program.handle, i, ARRAY_COUNT(attributeName),
                &attributeNameLength, &attributeSize, &attributeType, attributeName);

            GLuint attibuteLocation = glGetAttribLocation(program.handle, attributeName);

            program.vertexInputLayout.attributes.push_back(
                VertexShaderAttribute(static_cast<u8>(attibuteLocation), static_cast<u8>(attributeSize)));
        }
    }

    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 App::LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void RenderScreenFillQuad(App* app, const Framebuffer& aFBO) 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Program& programTexturedGeometry = app->programs[app->renderQuadProgramIdx];
    glUseProgram(programTexturedGeometry.handle);

    glBindVertexArray(app->vao);

    const char* uniformNames[] = { "uAlbedo", "uNormal", "uPosition", "uViewDir" };
    for (size_t i = 0; i < aFBO.GetColorAttachmentCount(); ++i)
    {
        GLint uniformLoc = glGetUniformLocation(programTexturedGeometry.handle, uniformNames[i]);
        glUniform1i(uniformLoc, i); // Use iteration index instead of 0
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, aFBO.GetColorAttachment(i));
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

void App::Init(App* app)
{
    glEnable(GL_DEPTH_TEST);

    // TODO: Initialize your resources here!
    
    // - vertex buffers
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // - element/index buffers
    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // - vaos
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);

    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3UV2), (void*)offsetof(Vertex3UV2, pos));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3UV2), (void*)offsetof(Vertex3UV2, uv));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // - programs (and retrieve uniform indices)
    app->renderQuadProgramIdx = LoadProgram(app, "Shaders/DEFERRED_RENDER_QUAD.glsl", "DEFERRED_RENDER_QUAD");
    Program& renderQuadProgram = app->programs[app->renderQuadProgramIdx];
    app->programUniformTexture = glGetUniformLocation(renderQuadProgram.handle, "uAlbedo");
    app->programUniformDebugMode = glGetUniformLocation(renderQuadProgram.handle, "uDebugMode");

    // - textures
    app->diceTexIdx = LoadTexture2D(app, "dice.png");

    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

    app->lightBlueTexIdx = LoadTexture2D(app, "color_light_blue.png");
    app->greenTexIdx = LoadTexture2D(app, "color_green.png");
    app->purpleTexIdx = LoadTexture2D(app, "color_purple.png");
    app->blueTexIdx = LoadTexture2D(app, "color_blue.png");
    app->lightGreenTexIdx = LoadTexture2D(app, "color_light_green.png");
    app->orangeTexIdx = LoadTexture2D(app, "color_orange.png");

    // Patrick Program

    app->renderGeometryProgramIdx = LoadProgram(app, "Shaders/DEFERRED_RENDER_GEOMETRY.glsl", "DEFERRED_RENDER_GEOMETRY");
    Program& renderGeometryProgram = app->programs[app->renderGeometryProgramIdx];
    app->patrickProgramUniformTexture = glGetUniformLocation(renderGeometryProgram.handle, "uAlbedo");

    app->forwardRenderingProgramIdx = LoadProgram(app, "Shaders/FORWARD_RENDER.glsl", "FORWARD_RENDER");
    Program& forwardRenderingProgram = app->programs[app->forwardRenderingProgramIdx];
    app->fwdPatrickProgramUniformTexture = glGetUniformLocation(forwardRenderingProgram.handle, "uTexture");

    app->lightSphereProgramIdx = LoadProgram(app, "Shaders/POINT_LIGHT_SPHERE.glsl", "POINT_LIGHT_SPHERE");
    
    app->patrickIdx = LoadModel(app, "Patrick/Patrick.obj");
    app->planeIdx = LoadModel(app, "Patrick/plane.obj");
    app->coneIdx = LoadModel(app, "Patrick/Cone.obj");
    app->cubeIdx = LoadModel(app, "Patrick/Cube.obj");
    app->cylinderIdx = LoadModel(app, "Patrick/Cylinder.obj");
    app->sphereIdx = LoadModel(app, "Patrick/Sphere.obj");
    app->torusIdx = LoadModel(app, "Patrick/Torus.obj");

    // Camera Configuration

    if (!app->worldCamera.created)
    {
        app->worldCamera.SetPosition(glm::vec3(10.0f, 8.5f, 11.0f));
        app->worldCamera.SetTarget(glm::vec3(0.0f, 0.0f, 0.0f));
        app->worldCamera.SetAspectRatio(static_cast<float>(app->displaySize.x) / static_cast<float>(app->displaySize.y));
        app->worldCamera.SetNearFar(0.1f, 1000.0f);
        app->worldCamera.SetUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
        app->worldCamera.SetVerticalFOV(60.0f);

        app->worldCamera.created = true;
    }
    
    //glm::mat4 view = app->worldCamera.ViewMatrix();
    //glm::mat4 projection = app->worldCamera.ProjectionMatrix();

    //glm::vec3 translation = glm::vec3(2.5f, 1.5f, -2.8f);
    //glm::vec3 scale = glm::vec3(0.45f);

    //glm::mat4 model = TransformPositionScale(translation, scale);
    //glm::mat4 MVP = projection * view * model;

    // Uniform Buffer
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);
    app->globalUBO = CreateConstantBuffer(app->maxUniformBufferSize);
    app->entityUBO = CreateConstantBuffer(app->maxUniformBufferSize);

    CreateLights();
    UpdateLights(app);

    //Buffer& entityUBO = app->entityUBO;

    MapBuffer(app->entityUBO, GL_WRITE_ONLY);
    CreateEntity(app, app->patrickIdx, 0, CreateTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    CreateEntity(app, app->planeIdx, app->lightGreenTexIdx, CreateTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    CreateEntity(app, app->coneIdx, app->purpleTexIdx, CreateTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    CreateEntity(app, app->cubeIdx, app->blueTexIdx, CreateTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    CreateEntity(app, app->cylinderIdx, app->orangeTexIdx, CreateTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    CreateEntity(app, app->sphereIdx, app->greenTexIdx, CreateTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    CreateEntity(app, app->torusIdx, app->lightBlueTexIdx, CreateTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    UnmapBuffer(app->entityUBO);

    if (!app->primaryFBO.Create(4, app->displaySize)) 
    {
        ELOG("[ERROR] The framebuffer was not created correctly.");
    }

    app->needsReinit = false;
}

void App::CreateEntity(App* app, const u32 aModelIdx, const u32 aTextureIdx, const glm::mat4& aWorldMatrix)
{
    Entity entity;
    AlignHead(app->entityUBO, app->uniformBlockAlignment);
    entity.entityBufferOffset = app->entityUBO.head;

    entity.worldMatrix = aWorldMatrix;
    entity.modelIdx = aModelIdx;
    entity.textureIdx = aTextureIdx;

    // Only push world matrix during creation
    PushMat4(app->entityUBO, entity.worldMatrix);
    // Reserve space for MVP (will be updated later)
    PushMat4(app->entityUBO, glm::identity<glm::mat4>());

    entity.entityBufferSize = app->entityUBO.head - entity.entityBufferOffset;
    app->entities.push_back(entity);
}

void App::Gui(App* app)
{
    Editor::Draw(app);
}

void TestFunction()
{
    OpenGLErrorGuard guard("TestFunction");

    // Trigger GL_INVALID_VALUE (0x0501) - Passing an out-of-range value
    // glPointSize(-10.0f); // Negative size is not allowed
}

void CameraMovement(App* app)
{
    // Camera rotation with right mouse button
    if (app->input.mouseButtons[RIGHT] == BUTTON_PRESSED)
    {
        float sensitivity = 0.1f;
        float deltaX = app->input.mouseDelta.x * sensitivity;
        float deltaY = app->input.mouseDelta.y * sensitivity;

        glm::vec3 position = app->worldCamera.GetPosition();
        glm::vec3 target = app->worldCamera.GetTarget();
        glm::vec3 up = app->worldCamera.GetUpVector();

        glm::vec3 forward = glm::normalize(target - position);
        glm::vec3 right = glm::normalize(glm::cross(forward, up));

        // Rotate forward vector based on mouse delta
        glm::mat4 yawRot = glm::rotate(glm::mat4(1.0f), glm::radians(-deltaX), up);
        forward = glm::vec3(yawRot * glm::vec4(forward, 0.0f));

        glm::mat4 pitchRot = glm::rotate(glm::mat4(1.0f), glm::radians(-deltaY), right);
        forward = glm::vec3(pitchRot * glm::vec4(forward, 0.0f));

        // Update target
        app->worldCamera.SetTarget(position + forward);
    }

    // WASDEQ movement
    glm::vec3 position = app->worldCamera.GetPosition();
    float baseSpeed = 10.0f * app->deltaTime;
    float speed = baseSpeed;

    glm::vec3 forward = glm::normalize(app->worldCamera.GetTarget() - position);
    glm::vec3 right = glm::normalize(glm::cross(forward, app->worldCamera.GetUpVector()));
    glm::vec3 up = app->worldCamera.GetUpVector();

    if (app->input.keys[K_W] == BUTTON_PRESSED)
    {
        position += forward * speed;
    }
    if (app->input.keys[K_S] == BUTTON_PRESSED)
    {
        position -= forward * speed;
    }

    if (app->input.keys[K_A] == BUTTON_PRESSED)
    {
        position -= right * speed;
    }

    if (app->input.keys[K_D] == BUTTON_PRESSED)
    {
        position += right * speed;
    }

    if (app->input.keys[K_Q] == BUTTON_PRESSED)
    {
        // Move down
        position -= up * speed;
    }

    if (app->input.keys[K_E] == BUTTON_PRESSED)
    {
        // Move up
        position += up * speed;
    }

    if (app->input.mouseButtons[RIGHT] == BUTTON_PRESSED) 
    {
        // Handle zoom using scroll wheel
        float zoomSpeed = 50.0f;
        position += forward * app->input.mouseScrollDeltaY * zoomSpeed * app->deltaTime;
    }

    // Update camera position and target
    app->worldCamera.SetPosition(position);
    app->worldCamera.SetTarget(position + forward);

    //// Only update existing entities' VP matrices
    //UpdateEntityUBO(app);
    //app->UpdateLights(app);
}

void App::Update(App* app)
{
#ifdef _DEBUG
    // Shader Hot Reload
    // You can handle app->input keyboard/mouse here
    for (u64 i = 0; i < app->programs.size(); ++i)
    {
        Program& program = app->programs[i];
        u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimestamp > program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            String programSource = ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = CreateProgramFromSource(app, programSource, programName);
            program.lastWriteTimestamp = currentTimestamp;
        }
    }
#endif

    {
        // Rotate Patrick

        static Entity* entity = &app->entities[0];
        static float rotationSpeed = glm::radians(45.0f); // 45 degrees per second

        float angle = rotationSpeed * app->deltaTime;
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

        entity->worldMatrix = entity->worldMatrix * rotation; // Apply local rotation
    }

    // Store previous camera state
    static glm::vec3 prevPosition = app->worldCamera.GetPosition();
    static glm::vec3 prevTarget = app->worldCamera.GetTarget();

    CameraMovement(app);
    UpdateEntityUBO(app);

    // Check if camera changed
    if (app->worldCamera.GetPosition() != prevPosition ||
        app->worldCamera.GetTarget() != prevTarget)
    {
        app->UpdateLights(app);

        // Store new state
        prevPosition = app->worldCamera.GetPosition();
        prevTarget = app->worldCamera.GetTarget();
    }

    // Debug keys
    if (app->input.keys[K_1] == BUTTON_PRESS) app->gBufferDebugMode = 0;
    if (app->input.keys[K_2] == BUTTON_PRESS) app->gBufferDebugMode = 1;
    if (app->input.keys[K_3] == BUTTON_PRESS) app->gBufferDebugMode = 2;
    if (app->input.keys[K_4] == BUTTON_PRESS) app->gBufferDebugMode = 3;
    if (app->input.keys[K_5] == BUTTON_PRESS) app->gBufferDebugMode = 4;
    if (app->input.keys[K_6] == BUTTON_PRESS) app->gBufferDebugMode = 5; // Add this

    // Handle rendering mode changes
    if (app->needsReinit)
    {
        CleanUp(app);    // Release existing resources
        Init(app);       // Re-initialize with new mode
        app->needsReinit = false;
    }
}

void App::OnResize(int width, int height)
{
    displaySize = vec2(width, height);

    primaryFBO.Clear();
    primaryFBO.Create(4, displaySize);

    worldCamera.SetAspectRatio(static_cast<float>(displaySize.x) / static_cast<float>(displaySize.y));

    UpdateEntityUBO(this);
}

void App::Render(App* app)
{
    switch (app->mode)
    {
        case Mode_Forward_Rendering:
        {
            // TODO: Draw your textured quad here!
            // - clear the framebuffer
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // - set the viewport
            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            // - set the blending state
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            Program& texturedMeshProgram = app->programs[app->forwardRenderingProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            // UNIFORM BUFFER:
            // void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->globalUBO.handle, 0, app->globalUBO.size);

            for (const auto& entity : app->entities)
            {
                // UNIFORM BUFFER:
                // void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->entityUBO.handle, entity.entityBufferOffset, entity.entityBufferSize);

                Model& model = app->models[entity.modelIdx];
                Mesh& mesh = app->meshes[model.meshIdx];

                for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                {
                    GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                    glBindVertexArray(vao);

                    u32 submeshMaterialIdx = model.materialIdx[i];
                    Material& submeshMaterial = app->materials[submeshMaterialIdx];

                    if (submeshMaterial.albedoTextureIdx > 0)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                        glUniform1i(app->fwdPatrickProgramUniformTexture, 0);
                    }
                    else 
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, app->textures[entity.textureIdx].handle);
                        glUniform1i(app->fwdPatrickProgramUniformTexture, 0);
                    }

                    Submesh& submesh = mesh.submeshes[i];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

                    glBindTexture(GL_TEXTURE_2D, 0);
                    glBindVertexArray(0);
                }

            }

            // ----------------------------------- Light Debug Geometry Pass ----------------------------------- //

            if (app->gBufferDebugMode == 0)
            {
                // Render light spheres
                glDisable(GL_BLEND);

                Program& lightSphereProgram = app->programs[app->lightSphereProgramIdx];
                glUseProgram(lightSphereProgram.handle);

                // Get camera matrices
                glm::mat4 view = app->worldCamera.ViewMatrix();
                glm::mat4 projection = app->worldCamera.ProjectionMatrix();

                // Bind sphere model's VAO
                Model& sphereModel = app->models[app->sphereIdx];
                Mesh& sphereMesh = app->meshes[sphereModel.meshIdx];
                Submesh& submesh = sphereMesh.submeshes[0];
                GLuint vao = FindVAO(sphereMesh, 0, lightSphereProgram);
                glBindVertexArray(vao);

                for (const auto& light : app->lights)
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
                        glBindVertexArray(0);
                    }
                }

                glBindVertexArray(0);
                glUseProgram(0);
            }

            break;
        }
        case Mode_Deferred_Rendering:
        {
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // ----------------------------------- Geometry Pass ----------------------------------- //

            glBindFramebuffer(GL_FRAMEBUFFER, app->primaryFBO.GetFramebufferHandle());

            std::vector<GLuint> drawBuffers;

            for (size_t i = 0; i < app->primaryFBO.GetColorAttachmentCount(); ++i)
            {
                drawBuffers.push_back(app->primaryFBO.GetColorAttachment(i));
            }

            glDrawBuffers(drawBuffers.size(), drawBuffers.data());

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Program& geometryProgram = app->programs[app->renderGeometryProgramIdx];
            glUseProgram(geometryProgram.handle);

            // Render all geometry to G-Buffer
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->globalUBO.handle, 0, app->globalUBO.size);

            for (const auto& entity : app->entities)
            {
                // UNIFORM BUFFER:
                // void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->entityUBO.handle, entity.entityBufferOffset, entity.entityBufferSize);

                Model& model = app->models[entity.modelIdx];
                Mesh& mesh = app->meshes[model.meshIdx];

                for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                {
                    GLuint vao = FindVAO(mesh, i, geometryProgram);
                    glBindVertexArray(vao);

                    u32 submeshMaterialIdx = model.materialIdx[i];
                    Material& submeshMaterial = app->materials[submeshMaterialIdx];

                    if (submeshMaterial.albedoTextureIdx > 0)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                        glUniform1i(app->fwdPatrickProgramUniformTexture, 0);
                    }
                    else
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, app->textures[entity.textureIdx].handle);
                        glUniform1i(app->fwdPatrickProgramUniformTexture, 0);
                    }

                    Submesh& submesh = mesh.submeshes[i];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                    
                    // Unbind texture & VAO after drawing
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glBindVertexArray(0);
                }
            }

            glUseProgram(0); // Unbind shader program

            // ----------------------------------- Depth Blit ----------------------------------- //

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, app->primaryFBO.GetFramebufferHandle());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(
                0, 0, app->displaySize.x, app->displaySize.y,
                0, 0, app->displaySize.x, app->displaySize.y,
                GL_DEPTH_BUFFER_BIT, GL_NEAREST
            );

            // ----------------------------------- Lighting Pass ----------------------------------- //

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            Program& quadProgram = app->programs[app->renderQuadProgramIdx];
            glUseProgram(quadProgram.handle);

            static const struct GBufferTexture 
            {
                GLenum textureUnit;
                GLuint textureID;
                const char* uniformName;
            } gBufferTextures[] = 
            {
                { GL_TEXTURE0, app->primaryFBO.GetColorAttachment(0), "uAlbedo"   },
                { GL_TEXTURE1, app->primaryFBO.GetColorAttachment(1), "uNormal"   },
                { GL_TEXTURE2, app->primaryFBO.GetColorAttachment(2), "uPosition" },
                { GL_TEXTURE3, app->primaryFBO.GetColorAttachment(3), "uViewDir"  },
                { GL_TEXTURE4, app->primaryFBO.GetDepthAttachment(),    "uDepth"    }
            };

            // Bind all textures in a loop
            for (const auto& tex : gBufferTextures) 
            {
                glActiveTexture(tex.textureUnit);
                glBindTexture(GL_TEXTURE_2D, tex.textureID);
                glUniform1i(glGetUniformLocation(quadProgram.handle, tex.uniformName),
                    tex.textureUnit - GL_TEXTURE0); // Convert to 0-based index
            }

            glUniform1i(app->programUniformDebugMode, (GLint)app->gBufferDebugMode);

            // Render fullscreen quad
            glBindVertexArray(app->vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            // Unbind everything after lighting pass
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0); // Unbind last active texture
            glUseProgram(0); // Unbind shader program

            // ----------------------------------- Light Debug Geometry Pass ----------------------------------- //

            if (app->gBufferDebugMode == 0) 
            {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glDepthMask(GL_FALSE);  // Prevent overwriting depth buffer
                glDisable(GL_BLEND);

                Program& lightSphereProgram = app->programs[app->lightSphereProgramIdx];
                glUseProgram(lightSphereProgram.handle);

                // Get camera matrices
                glm::mat4 view = app->worldCamera.ViewMatrix();
                glm::mat4 projection = app->worldCamera.ProjectionMatrix();

                // Bind sphere model's VAO
                Model& sphereModel = app->models[app->sphereIdx];
                Mesh& sphereMesh = app->meshes[sphereModel.meshIdx];
                Submesh& submesh = sphereMesh.submeshes[0];
                GLuint vao = FindVAO(sphereMesh, 0, lightSphereProgram);
                glBindVertexArray(vao);

                for (const auto& light : app->lights)
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
                        glBindVertexArray(0);
                    }
                }

                glUseProgram(0);
                glDepthMask(GL_TRUE);
            }

            break;
        }
    }
}

void App::CleanUp(App* app)
{
    ELOG("Cleaning Up Engine");

    // --- Delete Textures ---
    for (auto& texture : app->textures)
    {
        glDeleteTextures(1, &texture.handle);
    }
    app->textures.clear();

    // --- Delete Shader Programs ---
    for (auto& program : app->programs)
    {
        glDeleteProgram(program.handle);
    }
    app->programs.clear();

    // --- Delete Main VAO ---
    if (app->vao != 0)
    {
        glDeleteVertexArrays(1, &app->vao);
        app->vao = 0;
    }

    // --- Delete Element/Index Buffers ---
    if (app->embeddedElements != 0)
    {
        glDeleteBuffers(1, &app->embeddedElements);
        app->embeddedElements = 0;
    }

    // --- Delete Vertex Buffers ---
    if (app->embeddedVertices != 0)
    {
        glDeleteBuffers(1, &app->embeddedVertices);
        app->embeddedVertices = 0;
    }

    // --- Clean Up Meshes (VAOs, VBOs, and IBOs) ---
    for (auto& mesh : app->meshes)
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
    app->meshes.clear();

    // --- Clean Up Models and Materials ---
    app->models.clear();
    app->materials.clear();

    // --- Clean Up Uniform Buffers ---
    if (app->globalUBO.handle != 0)
    {
        glDeleteBuffers(1, &app->globalUBO.handle);
        app->globalUBO.handle = 0;
    }
    if (app->entityUBO.handle != 0)
    {
        glDeleteBuffers(1, &app->entityUBO.handle);
        app->entityUBO.handle = 0;
    }

    // --- Clean Up Entities ---
    app->entities.clear();

    // --- Clean Up FBO ---
    app->primaryFBO.Clear();

    app->lights.clear();
    app->gridLights.clear();
    app->defaultLights.clear();
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

void App::UpdateLights(App* app)
{
    MapBuffer(app->globalUBO, GL_WRITE_ONLY);
    PushVec3(app->globalUBO, app->worldCamera.GetPosition());
    PushUInt(app->globalUBO, app->lights.size());
    for (size_t i = 0; i < app->lights.size(); i++)
    {
        AlignHead(app->globalUBO, sizeof(vec4));
        Light& light = app->lights[i];
        PushUInt(app->globalUBO, static_cast<unsigned int>(light.type));
        PushVec3(app->globalUBO, light.color);
        PushVec3(app->globalUBO, light.direction);
        PushVec3(app->globalUBO, light.position);
        PushFloat(app->globalUBO, light.constant);
        PushFloat(app->globalUBO, light.linear);
        PushFloat(app->globalUBO, light.quadratic);
        PushFloat(app->globalUBO, light.specularStrength);
    }
    UnmapBuffer(app->globalUBO);
}

void App::RenderEntity(App* app, Entity entity, u32 entityIdx, u32 textureIdx, u32 textureProgramUniform, Program program)
{
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->entityUBO.handle, entity.entityBufferOffset, app->entityUBO.size);

    Model& model = app->models[entityIdx];
    Mesh& mesh = app->meshes[model.meshIdx];

    for (u32 i = 0; i < mesh.submeshes.size(); ++i) 
    {
        GLuint vao = FindVAO(mesh, 0, program);
        glBindVertexArray(vao);

        u32 submeshMaterialIdx = model.materialIdx[0];
        Material& submeshMaterial = app->materials[submeshMaterialIdx];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, app->textures[textureIdx].handle);
        glUniform1i(textureProgramUniform, 0);

        Submesh& submesh = mesh.submeshes[0];
        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
    }
}
