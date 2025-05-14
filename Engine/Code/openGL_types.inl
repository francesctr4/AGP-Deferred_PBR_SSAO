#pragma once
#include "platform.h"
#include <glad/glad.h>

// --- Vertex Data Structures ---
// Defines a vertex with 3D position and 2D UV coordinates
struct Vertex3UV2
{
    glm::vec3 pos;  // Vertex position in 3D space
    glm::vec2 uv;   // Texture coordinates
};

// Quad vertices for a full-screen rectangle (NDC coordinates)
const Vertex3UV2 vertices[] = 
{
    { glm::vec3(-1.0, -1.0, 0.0), glm::vec2(0.0, 0.0) }, // Bottom-left
    { glm::vec3(1.0, -1.0, 0.0), glm::vec2(1.0, 0.0) },  // Bottom-right
    { glm::vec3(1.0,  1.0, 0.0), glm::vec2(1.0, 1.0) },  // Top-right
    { glm::vec3(-1.0,  1.0, 0.0), glm::vec2(0.0, 1.0) }  // Top-left
};

// Triangle indices for quad rendering (two triangles)
const u16 indices[] = 
{
    0, 1, 2,  // First triangle
    0, 2, 3   // Second triangle
};

// --- Texture/Image Handling ---
// Raw image data container
struct Image
{
    void* pixels;       // Pixel data pointer
    glm::ivec2 size;    // Image dimensions (width, height)
    i32 nchannels;      // Color channels (e.g., 3 for RGB, 4 for RGBA)
    i32 stride;         // Row pitch in bytes
};

// GL texture resource
struct Texture
{
    GLuint handle;         // OpenGL texture ID
    std::string filepath;  // Source texture path
};

// --- Shader Configuration ---
// Attribute description for vertex shader input
struct VertexShaderAttribute
{
    u8 location;         // Shader location (layout = X)
    u8 componentCount;   // Components per attribute (e.g., 3 for vec3)

    VertexShaderAttribute(u8 location, u8 componentCount)
        : location(location), componentCount(componentCount) {}
};

// Complete vertex input layout for a shader program
struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;  // All input attributes
};

// Shader program container
struct Program
{
    GLuint handle;                           // OpenGL program ID
    std::string filepath;                    // Source shader path
    std::string programName;                 // Human-readable identifier
    u64 lastWriteTimestamp;                  // For hot-reloading shaders
    VertexShaderLayout vertexInputLayout;    // Expected vertex format
};

// --- Rendering Pipeline ---
// Rendering mode selector
enum Mode
{
    Mode_BlinnPhong_Forward_Rendering = 0,  // Forward Blinn-Phong Shading Pipeline
    Mode_BlinnPhong_Deferred_Rendering = 1, // Deferred Blinn-Phong Shading Pipeline
    Mode_PBR_Forward_Rendering = 2,         // Forward PBR Shading Pipeline
    Mode_PBR_Deferred_Rendering = 3,        // Deferred PBR Shading Pipeline
};

// --- Material System ---
// Surface material properties
struct Material
{
    std::string name;           // Material identifier
    glm::vec3 albedo;           // Base color
    glm::vec3 emissive;         // Self-illumination color
    f32 smoothness;             // Surface smoothness (0-1)

    // Texture indices in global texture array
    u32 albedoTextureIdx;       // Albedo/diffuse texture
    u32 emissiveTextureIdx;     // Emission texture
    u32 specularTextureIdx;     // Specular map
    u32 normalsTextureIdx;      // Normal map
    u32 bumpTextureIdx;         // Height/Bump map
};

// --- Vertex Buffers & Attributes ---
// Vertex buffer attribute description
struct VertexBufferAttribute
{
    u8 location;        // Shader location binding
    u8 componentCount;  // Number of components (1-4)
    u8 offset;          // Byte offset within vertex structure

    VertexBufferAttribute(u32 location, u32 componentCount, u32 offset)
        : location(location), componentCount(componentCount), offset(offset) {}
};

// Complete vertex buffer layout
struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute> attributes;  // Attribute list
    u8 stride;                                      // Total size of one vertex in bytes
};

// --- Mesh System ---
// Vertex Array Object container
struct VAO
{
    GLuint handle;          // OpenGL VAO ID
    GLuint programHandle;   // Linked shader program
};

// Mesh subset with own rendering parameters
struct Submesh
{
    VertexBufferLayout vertexBufferLayout;  // Vertex format description
    std::vector<f32> vertices;              // Raw vertex data
    std::vector<u32> indices;               // Index data
    u32 vertexOffset;                       // Offset in vertex buffer
    u32 indexOffset;                        // Offset in index buffer

    std::vector<VAO> vaos;                  // VAOs for different shader programs
};

// Complete mesh container
struct Mesh
{
    std::vector<Submesh> submeshes;  // Mesh components
    GLuint vertexBufferHandle;       // GL buffer for vertices
    GLuint indexBufferHandle;        // GL buffer for indices
};

// --- Scene Entities ---
// Reference to mesh and materials
struct Model
{
    u32 meshIdx;                   // Index in global mesh array
    std::vector<u32> materialIdx;  // Associated materials
};

// Scene entity with transform
struct Entity
{
    u32 modelIdx;           // Reference to Model
    u32 textureIdx;         // Main texture index

    u32 entityBufferOffset; // For instanced rendering
    u32 entityBufferSize;   // Data size in instance buffer

    glm::mat4 worldMatrix;  // Model-to-world transform
};

// --- Lighting System ---
// Light type enumeration
enum LightType
{
    LightType_Directional,  // Infinite-direction light
    LightType_Point         // Omnidirectional point light
};

// Light properties container
struct Light
{
    LightType type;         // Light classification
    glm::vec3 color;        // RGB intensity
    glm::vec3 direction;    // For directional lights
    glm::vec3 position;     // For point lights

    // Attenuation parameters (point lights)
    float constant;         // Constant attenuation
    float linear;           // Linear attenuation
    float quadratic;        // Quadratic attenuation

    float specularStrength; // Specular intensity multiplier
};