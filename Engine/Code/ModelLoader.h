#pragma once

#include "platform.h"
#include "engine.h"

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
	float smoothness;

	u32 albedoTextureIdx;
	u32 emissiveTextureIdx;
	u32 specularTextureIdx;
	u32 normalsTextureIdx;
	u32 bumpTextureIdx;
};

struct VertexBufferAttribute
{
	u32 index;    // Location in the shader
	u32 size;     // Number of components (e.g., 3 for vec3)
	u32 offset;   // Byte offset in the buffer layout

	VertexBufferAttribute(u32 index, u32 size, u32 offset)
		: index(index), size(size), offset(offset) {}
};

struct VertexBufferLayout 
{
	std::vector<VertexBufferAttribute> attributes;
	float stride;
};

struct Submesh 
{
	VertexBufferLayout vertexBufferLayout;
	std::vector<float> vertices;
	std::vector<u32> indices;
	u32 vertexOffset;
	u32 indexOffset;
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
