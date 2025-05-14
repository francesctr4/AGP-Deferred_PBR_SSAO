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

Cubemap::Cubemap(Cubemap&& other) noexcept
    : cubemapID(other.cubemapID),
    hdrTextureID(other.hdrTextureID),
    captureFBO(other.captureFBO),
    captureRBO(other.captureRBO)
{
    other.cubemapID = 0;
    other.hdrTextureID = 0;
    other.captureFBO = 0;
    other.captureRBO = 0;
}

Cubemap& Cubemap::operator=(Cubemap&& other) noexcept {
    if (this != &other) {
        ReleaseResources();

        cubemapID = other.cubemapID;
        hdrTextureID = other.hdrTextureID;
        captureFBO = other.captureFBO;
        captureRBO = other.captureRBO;

        other.cubemapID = 0;
        other.hdrTextureID = 0;
        other.captureFBO = 0;
        other.captureRBO = 0;
    }
    return *this;
}

Cubemap::~Cubemap() 
{
    ReleaseResources();
}

// Load HDR environment map and convert to cubemap
bool Cubemap::LoadFromHDR(App* app, const char* hdrPath, u32 conversionShaderIdx, int size)
{
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
    ConvertHDRToCubemap(app, cubemapID, conversionShaderIdx, size);

    // [WIP] PBR Cubemap IBL precalculations.

  //  // pbr: setup framebuffer
  //// ----------------------
  //  unsigned int captureFBO;
  //  unsigned int captureRBO;
  //  glGenFramebuffers(1, &captureFBO);
  //  glGenRenderbuffers(1, &captureRBO);

  //  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  //  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  //  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  //  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

  //  // pbr: load the HDR environment map
  //  // ---------------------------------
  //  stbi_set_flip_vertically_on_load(true);
  //  int width, height, nrComponents;
  //  float* data = stbi_loadf(FileSystem::getPath("resources/textures/hdr/newport_loft.hdr").c_str(), &width, &height, &nrComponents, 0);
  //  unsigned int hdrTexture;
  //  if (data)
  //  {
  //      glGenTextures(1, &hdrTexture);
  //      glBindTexture(GL_TEXTURE_2D, hdrTexture);
  //      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

  //      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //      stbi_image_free(data);
  //  }
  //  else
  //  {
  //      std::cout << "Failed to load HDR image." << std::endl;
  //  }

  //  // pbr: setup cubemap to render to and attach to framebuffer
  //  // ---------------------------------------------------------
  //  unsigned int envCubemap;
  //  glGenTextures(1, &envCubemap);
  //  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  //  for (unsigned int i = 0; i < 6; ++i)
  //  {
  //      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
  //  }
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //  // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
  //  // ----------------------------------------------------------------------------------------------
  //  glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  //  glm::mat4 captureViews[] =
  //  {
  //      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
  //      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
  //      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
  //      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
  //      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
  //      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
  //  };

  //  // pbr: convert HDR equirectangular environment map to cubemap equivalent
  //  // ----------------------------------------------------------------------
  //  equirectangularToCubemapShader.use();
  //  equirectangularToCubemapShader.setInt("equirectangularMap", 0);
  //  equirectangularToCubemapShader.setMat4("projection", captureProjection);
  //  glActiveTexture(GL_TEXTURE0);
  //  glBindTexture(GL_TEXTURE_2D, hdrTexture);

  //  glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
  //  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  //  for (unsigned int i = 0; i < 6; ++i)
  //  {
  //      equirectangularToCubemapShader.setMat4("view", captureViews[i]);
  //      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
  //      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //      renderCube();
  //  }
  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  //  // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
  //  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  //  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  //  // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
  //  // --------------------------------------------------------------------------------
  //  unsigned int irradianceMap;
  //  glGenTextures(1, &irradianceMap);
  //  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  //  for (unsigned int i = 0; i < 6; ++i)
  //  {
  //      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
  //  }
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  //  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  //  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

  //  // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
  //  // -----------------------------------------------------------------------------
  //  irradianceShader.use();
  //  irradianceShader.setInt("environmentMap", 0);
  //  irradianceShader.setMat4("projection", captureProjection);
  //  glActiveTexture(GL_TEXTURE0);
  //  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  //  glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
  //  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  //  for (unsigned int i = 0; i < 6; ++i)
  //  {
  //      irradianceShader.setMat4("view", captureViews[i]);
  //      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
  //      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //      renderCube();
  //  }
  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  //  // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
  //  // --------------------------------------------------------------------------------
  //  unsigned int prefilterMap;
  //  glGenTextures(1, &prefilterMap);
  //  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  //  for (unsigned int i = 0; i < 6; ++i)
  //  {
  //      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
  //  }
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //  // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
  //  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  //  // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
  //  // ----------------------------------------------------------------------------------------------------
  //  prefilterShader.use();
  //  prefilterShader.setInt("environmentMap", 0);
  //  prefilterShader.setMat4("projection", captureProjection);
  //  glActiveTexture(GL_TEXTURE0);
  //  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  //  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  //  unsigned int maxMipLevels = 5;
  //  for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
  //  {
  //      // reisze framebuffer according to mip-level size.
  //      unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
  //      unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
  //      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  //      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
  //      glViewport(0, 0, mipWidth, mipHeight);

  //      float roughness = (float)mip / (float)(maxMipLevels - 1);
  //      prefilterShader.setFloat("roughness", roughness);
  //      for (unsigned int i = 0; i < 6; ++i)
  //      {
  //          prefilterShader.setMat4("view", captureViews[i]);
  //          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

  //          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //          renderCube();
  //      }
  //  }
  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  //  // pbr: generate a 2D LUT from the BRDF equations used.
  //  // ----------------------------------------------------
  //  unsigned int brdfLUTTexture;
  //  glGenTextures(1, &brdfLUTTexture);

  //  // pre-allocate enough memory for the LUT texture.
  //  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
  //  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
  //  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //  // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
  //  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  //  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  //  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  //  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

  //  glViewport(0, 0, 512, 512);
  //  brdfShader.use();
  //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //  renderQuad();

  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

// Render skybox using specified shader
void Cubemap::RenderSkybox(App* app, u32 skyboxShaderIdx, u32 cubemapIdx, const glm::mat4& view, const glm::mat4& projection)
{
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
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapIdx);

    Cubemap::RenderCube();

    glDepthMask(GL_TRUE);
}

void Cubemap::CreateCube() 
{
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

void Cubemap::ConvertHDRToCubemap(App* app, u32& cubemapID, u32 conversionShaderIdx, int size) {
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

void Cubemap::ReleaseCube()
{
    if (cubeVAO)
    {
        glDeleteVertexArrays(1, &cubeVAO);
        cubeVAO = 0;
    }
    if (cubeVBO)
    {
        glDeleteBuffers(1, &cubeVBO);
        cubeVBO = 0;
    }
    cubeInitialized = false;
}
