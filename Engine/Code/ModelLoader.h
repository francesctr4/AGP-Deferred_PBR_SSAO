#pragma once

#include "platform.h"

#include <glad/glad.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

struct App;

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

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);
void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
u32 LoadModel(App* app, const char* filename);
