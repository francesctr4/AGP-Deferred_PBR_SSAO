//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "OpenGLErrorGuard.h"
#include "ModelLoader.h"

#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <format>

bool Framebuffer::CreateFBO(GLuint aAttachments, glm::vec2 displaySize)
{
    for (int i = 0; i < aAttachments; ++i)
    {
        // Color Attachment
        GLuint colorAttachment;
        glGenTextures(1, &colorAttachment);
        glBindTexture(GL_TEXTURE_2D, colorAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
            displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        attachments.push_back({ GL_COLOR_ATTACHMENT0 + i, colorAttachment });
    }

    // Depth Attachment
    GLuint depthAttachment;
    glGenTextures(1, &depthAttachment);
    glBindTexture(GL_TEXTURE_2D, depthAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
        displaySize.x, displaySize.y, 0,
        GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    depthHandle = depthAttachment;

    glGenFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);

    for (auto& attachment : attachments)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, attachment.first, attachment.second, 0);
    }

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthHandle, 0);

    GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            switch (framebufferStatus)
            {
            case GL_FRAMEBUFFER_UNDEFINED: ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED: ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
            default: ELOG("Unknown framebuffer status error");
            }
        }
    }

    std::vector<GLenum> drawBuffers;
    for (auto& [attachment, handle] : attachments) 
    {
        drawBuffers.push_back(attachment);
    }
    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
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

u32 LoadTexture2D(App* app, const char* filepath)
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

    Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx];
    glUseProgram(programTexturedGeometry.handle);

    glBindVertexArray(app->vao);

    size_t iteration = 0;
    const char* uniformNames[] = { "uAlbedo", "uNormal", "uPosition", "uViewDir" };
    for (const auto& texture : aFBO.attachments)
    {
        GLint uniformLoc = glGetUniformLocation(programTexturedGeometry.handle, uniformNames[iteration]);
        glUniform1i(uniformLoc, iteration); // Use iteration index instead of 0
        glActiveTexture(GL_TEXTURE0 + iteration);
        glBindTexture(GL_TEXTURE_2D, texture.second);

        ++iteration;
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

void Init(App* app)
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
    app->texturedGeometryProgramIdx = LoadProgram(app, "Shaders/RENDER_QUAD.glsl", "RENDER_QUAD");
    Program& texturedGeometryProgram = app->programs[app->texturedGeometryProgramIdx];
    app->programUniformTexture = glGetUniformLocation(texturedGeometryProgram.handle, "uAlbedo");
    app->programUniformDebugMode = glGetUniformLocation(texturedGeometryProgram.handle, "uDebugMode");

    // - textures
    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

    // Patrick Program

    app->texturedMeshProgramIdx = LoadProgram(app, "Shaders/RENDER_GEOMETRY.glsl", "RENDER_GEOMETRY");
    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    app->patrickProgramUniformTexture = glGetUniformLocation(texturedMeshProgram.handle, "uAlbedo");
    
    app->patrickIdx = LoadModel(app, "Patrick/Patrick.obj");
    app->planeIdx = LoadModel(app, "Patrick/plane.obj");

    // Camera Configuration

    app->worldCamera.SetPosition(glm::vec3(0.0f, 12.0f, 30.0f));
    app->worldCamera.SetTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    app->worldCamera.SetAspectRatio(static_cast<float>(app->displaySize.x) / static_cast<float>(app->displaySize.y));
    app->worldCamera.SetNearFar(0.1f, 1000.0f);
    app->worldCamera.SetUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
    app->worldCamera.SetVerticalFOV(60.0f);

    glm::mat4 view = app->worldCamera.ViewMatrix();
    glm::mat4 projection = app->worldCamera.ProjectionMatrix();

    glm::vec3 translation = glm::vec3(2.5f, 1.5f, -2.8f);
    glm::vec3 scale = glm::vec3(0.45f);

    glm::mat4 model = TransformPositionScale(translation, scale);
    glm::mat4 MVP = projection * view * model;

    // Uniform Buffer

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

    app->globalUBO = CreateConstantBuffer(app->maxUniformBufferSize);
    app->entityUBO = CreateConstantBuffer(app->maxUniformBufferSize);

    //MapBuffer(app->localParamsUBO, GL_WRITE_ONLY);
    //PushMat4(app->localParamsUBO, view);
    //PushMat4(app->localParamsUBO, projection);
    //UnmapBuffer(app->localParamsUBO);

    //MapBuffer(app->globalUBO, GL_WRITE_ONLY);
    //PushMat4(app->globalUBO, glm::identity<glm::mat4>());
    //glm::mat4 MVPMatrix = projection * view * glm::identity<glm::mat4>();
    //PushMat4(app->globalUBO, MVPMatrix);
    //UnmapBuffer(app->globalUBO);

    app->lights.push_back({ LightType_Directional, glm::vec3(0.5f, 0.0f, 0.0f), vec3(1.0f, 1.0f, -1.0f), vec3(0.0f) });
    app->lights.push_back({ LightType_Point, glm::vec3(0.2f, 0.2f, 0.2f), vec3(-1.0f, -1.0f, 1.0f), vec3(0.0f)});
    UpdateLights(app);

    Buffer& entityUBO = app->entityUBO;

    MapBuffer(app->entityUBO, GL_WRITE_ONLY);
    glm::mat4 VP = app->worldCamera.ProjectionMatrix() * app->worldCamera.ViewMatrix();

    for (int z = -2; z <= 2; ++z) 
    {
        for (int x = -2; x <= 2; ++x) 
        {
            CreateEntity(app, app->patrickIdx, VP, TransformPositionScale(glm::vec3(x * 6.0f, 0.0f, z * 6.0f), glm::vec3(1.0f)));
        }
    }

    CreateEntity(app, app->planeIdx, VP, TransformPositionScale(glm::vec3(0.0f), glm::vec3(1.0f)));

    UnmapBuffer(app->entityUBO);

    app->mode = Mode_Deferred_Shading;

    app->primaryFBO.CreateFBO(4, app->displaySize);
}

void CreateEntity(App* app, const u32 aModelIdx, const glm::mat4& aVP, const glm::mat4& aWorldMatrix) 
{
    Entity entity;
    AlignHead(app->entityUBO, app->uniformBlockAlignment);
    entity.entityBufferOffset = app->entityUBO.head;

    entity.worldMatrix = aWorldMatrix;
    entity.modelIndex = aModelIdx;

    PushMat4(app->entityUBO, entity.worldMatrix);
    PushMat4(app->entityUBO, aVP * entity.worldMatrix);

    entity.entityBufferSize = app->entityUBO.head - entity.entityBufferOffset;

    app->entities.push_back(entity);
}

void Gui(App* app)
{    
    // Set DockSpace Invisible Window Flags
    ImGuiWindowFlags window = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // Get Window Viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Set Window Parameters
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.0f);

    // Set Window Style Parameters
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Begin DockSpace Invisible Window with the flags
    ImGui::Begin("Dockspace", 0, window);

    // Apply Window Style Parameters
    ImGui::PopStyleVar(3);

    // Create DockSpace on the invisible window
    ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    // End DockSpace Window
    ImGui::End();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New")) { /* Handle new */ }
            if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Handle open */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Handle save */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { app->isRunning = false; }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About")) { /* Show about dialog */ }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::Text(app->mOpenGLInfo.c_str());
    ImGui::End();

    ImGui::Separator();

    ImGui::Begin("Lights");
    bool lightChanged = false;
    ImGui::Text("Lights");
    for (auto& light : app->lights) 
    {
        glm::vec3 checkVector;
        ImGui::PushID(&light);

        float color[3] = { light.color.x, light.color.y, light.color.z };
        ImGui::DragFloat3("Color", color, 0.01, 0.0, 1.0);
        checkVector = vec3(color[0], color[1], color[2]);

        if (checkVector != light.color) 
        {
            light.color = checkVector;
            lightChanged = true;
        }

        float direction[3] = { light.direction.x, light.direction.y, light.direction.z };
        ImGui::DragFloat3("Direction", direction, 0.01, -1.0, 1.0);
        checkVector = vec3(direction[0], direction[1], direction[2]);

        if (checkVector != light.direction)
        {
            light.direction = checkVector;
            lightChanged = true;
        }

        float position[3] = { light.position.x, light.position.y, light.position.z };
        ImGui::DragFloat3("Position", position, 0.1);
        checkVector = vec3(position[0], position[1], position[2]);

        if (checkVector != light.position)
        {
            light.position = checkVector;
            lightChanged = true;
        }

        ImGui::PopID();
        ImGui::Separator();
    }

    if (lightChanged) 
    {
        UpdateLights(app);
    }

    ImGui::End();

    // Error display window
    ImGui::Begin("Shader Errors", &app->showShaderErrors);
    {
        ImGui::SameLine();
        ImGui::Text("%d error(s)", static_cast<int>(app->shaderErrors.size()));

        ImGui::BeginChild("ErrorScroll");
        for (const auto& error : app->shaderErrors)
        {
            ImGui::TextUnformatted(error.c_str());
            ImGui::Separator();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void TestFunction()
{
    OpenGLErrorGuard guard("TestFunction");

    // Trigger GL_INVALID_VALUE (0x0501) - Passing an out-of-range value
    // glPointSize(-10.0f); // Negative size is not allowed
}

void PushCameraUniforms(App* app)
{
    MapBuffer(app->entityUBO, GL_WRITE_ONLY);
    glm::mat4 VP = app->worldCamera.ProjectionMatrix() * app->worldCamera.ViewMatrix();

    for (int z = -2; z <= 2; ++z)
    {
        for (int x = -2; x <= 2; ++x)
        {
            CreateEntity(app, app->patrickIdx, VP, TransformPositionScale(glm::vec3(x * 6.0f, 0.0f, z * 6.0f), glm::vec3(1.0f)));
        }
    }

    CreateEntity(app, app->planeIdx, VP, TransformPositionScale(glm::vec3(0.0f), glm::vec3(1.0f)));

    UnmapBuffer(app->entityUBO);
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

        PushCameraUniforms(app);
    }

    // WASD movement
    glm::vec3 position = app->worldCamera.GetPosition();
    float speed = 5.0f * app->deltaTime; // Adjust speed as needed

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

    // Update camera position and target
    app->worldCamera.SetPosition(position);
    app->worldCamera.SetTarget(position + forward); // Keep looking ahead
}

void Update(App* app)
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

    TestFunction();

    CameraMovement(app);
}

void App::OnResize(int width, int height) 
{
    displaySize = vec2(width, height);

    primaryFBO.Clean();
    primaryFBO.CreateFBO(4, displaySize);

    worldCamera.SetAspectRatio(static_cast<float>(displaySize.x) / static_cast<float>(displaySize.y));

    MapBuffer(entityUBO, GL_WRITE_ONLY);
    glm::mat4 VP = worldCamera.ProjectionMatrix() * worldCamera.ViewMatrix();

    for (int z = -2; z <= 2; ++z)
    {
        for (int x = -2; x <= 2; ++x)
        {
            CreateEntity(this, patrickIdx, VP, TransformPositionScale(glm::vec3(x * 6.0f, 0.0f, z * 6.0f), glm::vec3(1.0f)));
        }
    }

    CreateEntity(this, planeIdx, VP, TransformPositionScale(glm::vec3(0.0f), glm::vec3(1.0f)));

    UnmapBuffer(entityUBO);
}

void Render(App* app)
{
    switch (app->mode)
    {
        case Mode_TexturedQuad:
        {
            // TODO: Draw your textured quad here!
            // - clear the framebuffer
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // - set the viewport
            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            // - set the blending state
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // - bind the program 
            Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx];
            glUseProgram(programTexturedGeometry.handle);

            // - (...and make its texture sample from unit 0)
            glUniform1i(app->programUniformTexture, 0);

            // - bind the texture into unit 0
            glActiveTexture(GL_TEXTURE0);
            GLuint textureHandle = app->textures[app->diceTexIdx].handle;
            glBindTexture(GL_TEXTURE_2D, textureHandle);

            // - bind the vao
            glBindVertexArray(app->vao);

            // - glDrawElements() !!!
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            // - unbind the vao
            glBindVertexArray(0);

            // - unbind the program
            glUseProgram(0);

            break;
        }
        case Mode_Forward_Geometry:
        {
            // TODO: Draw your textured quad here!
            // - clear the framebuffer
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // - set the viewport
            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            // - set the blending state
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            Model& model = app->models[app->patrickIdx];
            Mesh& mesh = app->meshes[model.meshIdx];

            for (u32 i = 0; i < mesh.submeshes.size(); ++i)
            {
                GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                glBindVertexArray(vao);

                u32 submeshMaterialIdx = model.materialIdx[i];
                Material& submeshMaterial = app->materials[submeshMaterialIdx];

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                glUniform1i(app->patrickProgramUniformTexture, 0);

                Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
            }

            break;
        }
        case Mode_Forward_Geometry_UBO:
        {
            // TODO: Draw your textured quad here!
            // - clear the framebuffer
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // - set the viewport
            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            // - set the blending state
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            // UNIFORM BUFFER:
            // void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->globalUBO.handle, 0, app->globalUBO.size);

            for (const auto& entity : app->entities)
            {
                // UNIFORM BUFFER:
                // void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->entityUBO.handle, entity.entityBufferOffset, app->entityUBO.size);

                Model& model = app->models[app->patrickIdx];
                Mesh& mesh = app->meshes[model.meshIdx];

                for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                {
                    GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                    glBindVertexArray(vao);

                    u32 submeshMaterialIdx = model.materialIdx[i];
                    Material& submeshMaterial = app->materials[submeshMaterialIdx];

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                    glUniform1i(app->patrickProgramUniformTexture, 0);

                    Submesh& submesh = mesh.submeshes[i];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                }
            }

            Model& model = app->models[app->planeIdx];
            Mesh& mesh = app->meshes[model.meshIdx];

            GLuint vao = FindVAO(mesh, 0, texturedMeshProgram);
            glBindVertexArray(vao);

            u32 submeshMaterialIdx = model.materialIdx[0];
            Material& submeshMaterial = app->materials[submeshMaterialIdx];

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->textures[app->whiteTexIdx].handle);
            glUniform1i(app->patrickProgramUniformTexture, 0);

            Submesh& submesh = mesh.submeshes[0];
            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

            break;
        }
        case Mode_Deferred_Shading:
        {
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // ==================== GEOMETRY PASS ====================
            glBindFramebuffer(GL_FRAMEBUFFER, app->primaryFBO.handle);

            std::vector<GLuint> drawBuffers;
            for (auto& [attachment, handle] : app->primaryFBO.attachments)
            {
                drawBuffers.push_back(handle);
            }
            glDrawBuffers(drawBuffers.size(), drawBuffers.data());

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Program& geometryProgram = app->programs[app->texturedMeshProgramIdx];
            glUseProgram(geometryProgram.handle);

            // Render all geometry to G-Buffer
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->globalUBO.handle, 0, app->globalUBO.size);

            for (const auto& entity : app->entities)
            {
                // UNIFORM BUFFER:
                // void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->entityUBO.handle, entity.entityBufferOffset, entity.entityBufferSize);

                Model& model = app->models[app->patrickIdx];
                Mesh& mesh = app->meshes[model.meshIdx];

                for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                {
                    GLuint vao = FindVAO(mesh, i, geometryProgram);
                    glBindVertexArray(vao);

                    u32 submeshMaterialIdx = model.materialIdx[i];
                    Material& submeshMaterial = app->materials[submeshMaterialIdx];

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                    glUniform1i(app->patrickProgramUniformTexture, 0);

                    Submesh& submesh = mesh.submeshes[i];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                    
                    // Unbind texture & VAO after drawing
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glBindVertexArray(0);
                }
            }

            Model& model = app->models[app->planeIdx];
            Mesh& mesh = app->meshes[model.meshIdx];

            GLuint vao = FindVAO(mesh, 0, geometryProgram);
            glBindVertexArray(vao);

            u32 submeshMaterialIdx = model.materialIdx[0];
            Material& submeshMaterial = app->materials[submeshMaterialIdx];

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->textures[app->whiteTexIdx].handle);
            glUniform1i(app->patrickProgramUniformTexture, 0);

            Submesh& submesh = mesh.submeshes[0];
            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

            // Unbind after drawing
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
            glUseProgram(0); // Unbind shader program

            // Lighting Pass ------------------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            Program& quadProgram = app->programs[app->texturedGeometryProgramIdx];
            glUseProgram(quadProgram.handle);

            // Bind G-Buffer textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->primaryFBO.attachments[0].second); // Albedo
            glUniform1i(glGetUniformLocation(quadProgram.handle, "uAlbedo"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, app->primaryFBO.attachments[1].second); // Normal
            glUniform1i(glGetUniformLocation(quadProgram.handle, "uNormal"), 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, app->primaryFBO.attachments[2].second); // Position
            glUniform1i(glGetUniformLocation(quadProgram.handle, "uPosition"), 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, app->primaryFBO.attachments[3].second); // ViewDir
            glUniform1i(glGetUniformLocation(quadProgram.handle, "uViewDir"), 3);

            // Render fullscreen quad
            glBindVertexArray(app->vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            // Unbind everything after lighting pass
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0); // Unbind last active texture
            glUseProgram(0); // Unbind shader program

            break;
        }

        default:;
    }
}

void CleanUp(App* app)
{
    ELOG("Cleaning Up Engine");

    // TODO: Deinitialize your resources here!
    
    // - delete textures
    for (auto& texture : app->textures)
    {
        glDeleteTextures(1, &texture.handle);
    }
    app->textures.clear();

    // - delete shader programs
    for (auto& program : app->programs)
    {
        glDeleteProgram(program.handle);
    }
    app->programs.clear();

    // - delete vao
    if (app->vao != 0)
    {
        glDeleteVertexArrays(1, &app->vao);
        app->vao = 0;
    }

    // - delete element/index buffers
    if (app->embeddedElements != 0)
    {
        glDeleteBuffers(1, &app->embeddedElements);
        app->embeddedElements = 0;
    }

    // - delete vertex buffers
    if (app->embeddedVertices != 0)
    {
        glDeleteBuffers(1, &app->embeddedVertices);
        app->embeddedVertices = 0;
    }

    app->primaryFBO.Clean();
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
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

void UpdateLights(App* app)
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
    }
    UnmapBuffer(app->globalUBO);
}

void RenderEntity(App* app, Entity entity, u32 entityIdx, u32 textureIdx, u32 textureProgramUniform, Program program)
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
