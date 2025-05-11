#include "Cubemap.h"

#include "ImageLoader.h"
#include "engine.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <glm/gtc/matrix_transform.hpp>

// Initialize static members
GLuint Cubemap::cubeVAO = 0;
GLuint Cubemap::cubeVBO = 0;
bool Cubemap::cubeInitialized = false;

Cubemap::Cubemap() : cubemapID(0), hdrTextureID(0), captureFBO(0), captureRBO(0) {
    
}

Cubemap::~Cubemap() {
    ReleaseResources();
}

// Load HDR environment map and convert to cubemap
bool Cubemap::LoadFromHDR(App* app, const char* hdrPath, u32 conversionShaderIdx, int size) 
{
    InitializeCube();

    // Load HDR texture
    int width, height, nrComponents;
    float* data = stbi_loadf(hdrPath, &width, &height, &nrComponents, 0);
    if (!data) return false;

    // Create HDR texture
    glGenTextures(1, &hdrTextureID);
    glBindTexture(GL_TEXTURE_2D, hdrTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    SetTextureParameters(GL_TEXTURE_2D);
    stbi_image_free(data);

    // Create cubemap
    cubemapID = CreateCubemapTexture(size);

    // Convert HDR to cubemap
    ConvertHDRToCubemap(app, conversionShaderIdx, size);

    return true;
}

// Render skybox using specified shader

void Cubemap::RenderSkybox(App* app, u32 skyboxShaderIdx, const glm::mat4& view, const glm::mat4& projection) {
    glDepthMask(GL_FALSE);
    Program& skyboxProgram = app->programs[skyboxShaderIdx];
    glUseProgram(skyboxProgram.handle);

    // Set up matrices (remove translation from view matrix)
    glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(view));
    GLint viewLoc = glGetUniformLocation(skyboxProgram.handle, "view");
    GLint projectionLoc = glGetUniformLocation(skyboxProgram.handle, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewWithoutTranslation));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Bind cubemap and render
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    RenderCube();

    glDepthMask(GL_TRUE);
}

void Cubemap::InitializeCube() {
    if (cubeInitialized) return;

    float vertices[] = {
        // Back
        -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
        // Front
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        // Left
        -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        // Right
        1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        // Bottom
        -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f,
        // Top
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    cubeInitialized = true;
}

void Cubemap::ConvertHDRToCubemap(App* app, u32 conversionShaderIdx, int size) {
    // Set up framebuffer
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // Set up projection and view matrices
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
    };

    // Convert HDR equirectangular to cubemap
    Program& conversionProgram = app->programs[conversionShaderIdx];
    glUseProgram(conversionProgram.handle);

    glUniform1i(glGetUniformLocation(conversionProgram.handle, "equirectangularMap"), 0);
    glUniformMatrix4fv(glGetUniformLocation(conversionProgram.handle, "projection"),
        1, GL_FALSE, glm::value_ptr(captureProjection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTextureID);

    glViewport(0, 0, size, size);
    for (unsigned int i = 0; i < 6; ++i) {
        glUniformMatrix4fv(glGetUniformLocation(conversionProgram.handle, "view"),
            1, GL_FALSE, glm::value_ptr(captureViews[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
    }

    // Cleanup conversion resources
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteFramebuffers(1, &captureFBO);
}

GLuint Cubemap::CreateCubemapTexture(int size) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (GLuint i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            size, size, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    SetTextureParameters(GL_TEXTURE_CUBE_MAP);
    return textureID;
}

void Cubemap::SetTextureParameters(GLenum target) {
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (target == GL_TEXTURE_CUBE_MAP)
        glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Cubemap::RenderCube() {
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Cubemap::ReleaseResources() {
    if (cubemapID) glDeleteTextures(1, &cubemapID);
    if (hdrTextureID) glDeleteTextures(1, &hdrTextureID);
    if (captureFBO) glDeleteFramebuffers(1, &captureFBO);
    if (captureRBO) glDeleteRenderbuffers(1, &captureRBO);
}
