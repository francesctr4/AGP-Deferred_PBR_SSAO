#pragma once

#include "platform.h"
#include <glad/glad.h>

struct App;

struct Vertex3UV2
{
    glm::vec3 pos;
    glm::vec2 uv;
};

const Vertex3UV2 vertices[] = {
    { glm::vec3(-1.0, -1.0, 0.0), glm::vec2(0.0, 0.0) }, // bottom-left vertex
    { glm::vec3(1.0, -1.0, 0.0), glm::vec2(1.0, 0.0) }, // bottom-right vertex
    { glm::vec3(1.0,  1.0, 0.0), glm::vec2(1.0, 1.0) }, // top-right vertex
    { glm::vec3(-1.0,  1.0, 0.0), glm::vec2(0.0, 1.0) }  // top-left vertex
};

const u16 indices[] = {
    0, 1, 2,  
    0, 2, 3   
};

struct Image
{
    void* pixels;
    glm::ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;

    VertexShaderAttribute(u8 location, u8 componentCount)
        : location(location), componentCount(componentCount) {}
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexShaderLayout vertexInputLayout;
};

enum Mode
{
	Mode_Forward_Rendering = 0,
	Mode_Deferred_Rendering = 1,
};

struct Material
{
	std::string name;
	glm::vec3 albedo;
	glm::vec3 emissive;
	f32 smoothness;

	u32 albedoTextureIdx;
	u32 emissiveTextureIdx;
	u32 specularTextureIdx;
	u32 normalsTextureIdx;
	u32 bumpTextureIdx;
};

struct VertexBufferAttribute
{
	u8 location;    // Location in the shader
	u8 componentCount;     // Number of components (e.g., 3 for vec3)
	u8 offset;   // Byte offset in the buffer layout

	VertexBufferAttribute(u32 location, u32 componentCount, u32 offset)
		: location(location), componentCount(componentCount), offset(offset) {}
};

struct VertexBufferLayout
{
	std::vector<VertexBufferAttribute> attributes;
	u8 stride;
};

struct VAO
{
	GLuint handle;
	GLuint programHandle;
};

struct Submesh
{
	VertexBufferLayout vertexBufferLayout;
	std::vector<f32> vertices;
	std::vector<u32> indices;
	u32 vertexOffset;
	u32 indexOffset;

	std::vector<VAO> vaos;
};

struct Mesh
{
	std::vector<Submesh> submeshes;
	GLuint vertexBufferHandle;
	GLuint indexBufferHandle;
};

struct Model
{
	u32 meshIdx;
	std::vector<u32> materialIdx;
};

struct Entity
{
	u32 modelIdx;
	u32 textureIdx;

	u32 entityBufferOffset;
	u32 entityBufferSize;

	glm::mat4 worldMatrix;
};

enum LightType 
{
	LightType_Directional,
	LightType_Point
};

struct Light
{
	LightType type;
	glm::vec3 color;
	glm::vec3 direction;
	glm::vec3 position;
};

struct Framebuffer 
{
	GLuint handle;
	GLuint depthHandle;
	std::vector<std::pair<GLenum, GLuint>> attachments;

	bool CreateFBO(GLuint aAttachments, glm::vec2 displaySize);

	void Clean() 
	{
		// Unbind primaryFBO to safely modify it
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Delete all color attachments
		for (auto& [attachment, handle] : attachments)
		{
			glDeleteTextures(1, &handle);
		}
		attachments.clear();

		// Delete depth attachment (if it exists)
		if (depthHandle != 0)
		{
			glDeleteTextures(1, &depthHandle);
			depthHandle = 0;
		}

		// Delete the framebuffer itself
		if (handle != 0)
		{
			glDeleteFramebuffers(1, &handle);
			handle = 0;
		}
	}

	GLuint GetTextureAttachment(int index) {
		if (index >= 0 && index < attachments.size()) {
			return attachments[index].second;
		}
		return 0;
	}

	// Returns the depth texture handle
	GLuint GetDepthAttachment() {
		return depthHandle;
	}
};