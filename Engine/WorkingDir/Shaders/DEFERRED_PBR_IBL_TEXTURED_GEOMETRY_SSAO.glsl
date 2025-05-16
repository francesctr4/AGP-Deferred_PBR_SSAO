///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef DEFERRED_PBR_IBL_TEXTURED_GEOMETRY_SSAO

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBitangent;

struct Light 
{
	int type;
	vec3 color;
	vec3 direction;
	vec3 position;

	float constant;
    float linear;
    float quadratic;
    float specularStrength;
};

layout(binding = 0, std140) uniform globalUBO
{
	vec3 uCameraPosition;
	int uLightCount;
	Light uLight[800];
};

layout(binding = 1, std140) uniform entityUBO
{
	mat4 uWorldMatrix;
    mat4 uViewMatrix;
	mat4 uProjectionMatrix;
};

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vViewDir;
out vec3 vTangent;
out vec3 vBitangent;

void main()
{
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0f));

	mat3 normalMatrix = mat3(transpose(inverse(uWorldMatrix)));
    vNormal = normalMatrix * aNormal;
    vTangent = normalMatrix * aTangent;
    vBitangent = normalMatrix * aBitangent;

	vTexCoord = aTexCoord;
	vViewDir = uCameraPosition - vPosition;

	gl_Position = uProjectionMatrix * uViewMatrix * uWorldMatrix * vec4(aPosition, 1.0f);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vViewDir;
in vec3 vTangent;
in vec3 vBitangent;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uMetallic;
uniform sampler2D uRoughness;

layout(location=0) out vec4 oAlbedoRoughness;
layout(location=1) out vec4 oNormalMetallic;
layout(location=2) out vec4 oPosition;
layout(location=3) out vec4 oViewDir;

void main()
{
	// Sample textures
    vec3 albedo = texture(uAlbedo, vTexCoord).rgb;
    float metallic = texture(uMetallic, vTexCoord).r;
    float roughness = texture(uRoughness, vTexCoord).r;
    
    // Normal map calculation
    mat3 TBN = mat3(normalize(vTangent), 
                   normalize(vBitangent), 
                   normalize(vNormal));
    vec3 normal = texture(uNormal, vTexCoord).xyz * 2.0 - 1.0;
    normal = normalize(TBN * normal);
    
    // Store in G-buffer
    oAlbedoRoughness = vec4(albedo, roughness);
    oNormalMetallic = vec4(normal * 0.5 + 0.5, metallic); // Pack normal
    oPosition = vec4(vPosition, 1.0);
    oViewDir = vec4(vViewDir, 1.0);
}

#endif
#endif