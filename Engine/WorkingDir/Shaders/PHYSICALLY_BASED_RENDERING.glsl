///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef PHYSICALLY_BASED_RENDERING

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBitangent;

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

layout(binding = 0, std140) uniform globalUBO
{
	vec3 uCameraPosition;
	int uLightCount;
	BaseLight uLight[800];
};

layout(binding = 1, std140) uniform entityUBO
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vViewDir;

void main()
{
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0f));
	vNormal = mat3(transpose(inverse(uWorldMatrix))) * aNormal;
	vTexCoord = aTexCoord;
	vViewDir = uCameraPosition - vPosition;

	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0f);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

#define PI 3.1415926535897932384626433832795f

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vViewDir;

out vec4 FragColor;

struct PBRLight 
{
    vec4 PosDir;   // if w == 1 position, else direction
    vec3 Intensity;
};

struct PBRMaterial
{
    float Roughness;
    bool IsMetal;
    vec3 Color;
    bool IsAlbedo;
};

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

struct DirectionalLight {
    BaseLight Base;
    vec3 Direction;
};

struct PointLight {
    BaseLight Base;
    vec3 LocalPos;
};

uniform DirectionalLight gDirectionalLight;
uniform PointLight gPointLights[16]; // Adjust array size as needed
uniform int gNumPointLights;

layout(binding = 0) uniform sampler2D gAlbedo;
layout(binding = 1) uniform sampler2D gRoughness;
layout(binding = 2) uniform sampler2D gMetallic;
layout(binding = 3) uniform sampler2D gNormalMap;

uniform PBRMaterial gPBRmaterial;

vec3 schlickFresnel(float vDotH, vec3 Albedo)
{
    vec3 F0 = vec3(0.04);    
    
    if (gPBRmaterial.IsAlbedo) {
        float Metallic = texture(gMetallic, vTexCoord.xy).x;
        F0 = mix(F0, Albedo, Metallic);
    } else {
        if (gPBRmaterial.IsMetal) {
	        F0 = gPBRmaterial.Color;
	    }
    }

    vec3 ret = F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5);

    return ret;
}

float GetRoughness()
{
    if (gPBRmaterial.IsAlbedo) {
        return texture(gRoughness, vTexCoord.xy).x;
    } else {
        return gPBRmaterial.Roughness;
    }
}

float geomSmith(float dp, float Roughness)
{
    float k = (Roughness + 1.0) * (Roughness + 1.0) / 8.0;
    float denom = dp * (1 - k) + k;
    return dp / denom;
}

float ggxDistribution(float nDotH, float Roughness)
{
    float alpha2 = Roughness * Roughness * Roughness * Roughness;
    float d = nDotH * nDotH * (alpha2 - 1) + 1;
    float ggxdistrib = alpha2 / (PI * d * d);
    return ggxdistrib;
}

vec3 CalcPBRLighting(BaseLight Light, vec3 PosDir, bool IsDirLight, vec3 Normal)
{
    vec3 LightIntensity = Light.Color * Light.DiffuseIntensity;

    vec3 l = vec3(0.0);

    if (IsDirLight) {
        l = -PosDir.xyz;
    } else {
        l = PosDir - vPosition;
        float LightToPixelDist = length(l);
        l = normalize(l);
        LightIntensity /= (LightToPixelDist * LightToPixelDist);
    }

    vec3 n = Normal;
    vec3 v = normalize(vViewDir);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3 fLambert = vec3(0.0);

    if (!gPBRmaterial.IsMetal) {
        if (gPBRmaterial.IsAlbedo) {
            fLambert = pow(texture(gAlbedo, vTexCoord.xy).xyz, vec3(2.2));
        } else {
            fLambert = gPBRmaterial.Color;
        }
    }

    vec3 F = schlickFresnel(vDotH, fLambert);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    float Roughness = GetRoughness();

    vec3 SpecBRDF_nom  = ggxDistribution(nDotH, Roughness) *
                         F *
                         geomSmith(nDotL, Roughness) *
                         geomSmith(nDotV, Roughness);

    float SpecBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 SpecBRDF = SpecBRDF_nom / SpecBRDF_denom;

    vec3 DiffuseBRDF = kD * fLambert / PI;

    vec3 FinalColor = (DiffuseBRDF + SpecBRDF) * LightIntensity * nDotL;

    return FinalColor;
}

vec3 CalcPBRDirectionalLight(vec3 Normal)
{
    return CalcPBRLighting(gDirectionalLight.Base, gDirectionalLight.Direction, true, Normal);
}


vec3 CalcPBRPointLight(PointLight l, vec3 Normal)
{
    return CalcPBRLighting(l.Base, l.LocalPos, false, Normal);
}

vec4 CalcTotalPBRLighting()
{
    vec3 Normal = normalize(vNormal);

    vec3 TotalLight = CalcPBRDirectionalLight(Normal);

    for (int i = 0 ;i < gNumPointLights ;i++) 
    {
        TotalLight += CalcPBRPointLight(gPointLights[i], Normal);
    }

    // HDR tone mapping
    TotalLight = TotalLight / (TotalLight + vec3(1.0));

    // Gamma correction
    vec4 FinalLight = vec4(pow(TotalLight, vec3(1.0/2.2)), 1.0);

    return FinalLight;
}

void main() 
{
    FragColor = CalcTotalPBRLighting();
}

#endif
#endif