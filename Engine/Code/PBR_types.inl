#include "platform.h"
#include "openGL_types.inl"

class PBRMaterial 
{
public:
    // Textures
    unsigned int AlbedoTexture = 0;
    unsigned int RoughnessTexture = 0;
    unsigned int MetallicTexture = 0;
    unsigned int NormalTexture = 0;

    // Material properties
    glm::vec3 Color = glm::vec3(0.7f);
    float Roughness = 0.5f;
    bool IsMetal = false;
    bool UseAlbedoTexture = false;

    // Constructor
    PBRMaterial() = default;

    //void SetUniforms(Shader& shader) const 
    //{
    //    shader.SetBool("gPBRmaterial.IsAlbedo", UseAlbedoTexture);
    //    shader.SetBool("gPBRmaterial.IsMetal", IsMetal);
    //    shader.SetFloat("gPBRmaterial.Roughness", Roughness);
    //    shader.SetVec3("gPBRmaterial.Color", Color);

    //    // Bind textures
    //    shader.SetInt("gAlbedo", 0);
    //    glActiveTexture(GL_TEXTURE0);
    //    glBindTexture(GL_TEXTURE_2D, AlbedoTexture);

    //    shader.SetInt("gRoughness", 1);
    //    glActiveTexture(GL_TEXTURE1);
    //    glBindTexture(GL_TEXTURE_2D, RoughnessTexture);

    //    shader.SetInt("gMetallic", 2);
    //    glActiveTexture(GL_TEXTURE2);
    //    glBindTexture(GL_TEXTURE_2D, MetallicTexture);

    //    shader.SetInt("gNormalMap", 3);
    //    glActiveTexture(GL_TEXTURE3);
    //    glBindTexture(GL_TEXTURE_2D, NormalTexture);
    //}
};

struct PBRLight 
{
    glm::vec4 PosDir;   // if w == 1 position, else direction
    glm::vec3 Intensity;
};