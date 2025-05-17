///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef DEFERRED_PBR_IBL_TEXTURED_QUAD_SSAO

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location=0) out vec4 FragColor;

in vec2 vTexCoord;

// Material Parameters
layout(binding = 0) uniform sampler2D gAlbedoRoughness; // RGB: Albedo, A: Roughness
layout(binding = 1) uniform sampler2D gNormalMetallic;  // RGB: Normal, A: Metallic
layout(binding = 2) uniform sampler2D gPosition;
layout(binding = 3) uniform sampler2D gViewDir;
layout(binding = 4) uniform sampler2D gDepth;
layout(binding = 5) uniform sampler2D aoMap;

// IBL
layout(binding = 6) uniform samplerCube irradianceMap;
layout(binding = 7) uniform samplerCube prefilterMap;
layout(binding = 8) uniform sampler2D brdfLUT;

uniform int gDebugMode;

// lights
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

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   
// ----------------------------------------------------------------------------
float linearizeDepth(float depth, float near, float far) 
{
    // Convert depth to view-space Z
    float z = depth * 2.0 - 1.0; 
    float viewZ = (2.0 * near * far) / (far + near - z * (far - near));
    
    // Normalize to [0,1] range and invert. White=close, Black=far
    return 1.0 - ((viewZ - near) / (far - near));
}
// ----------------------------------------------------------------------------
void main()
{		
    // Retrieve data from G-buffer
    vec3 worldPos = texture(gPosition, vTexCoord).rgb;
    vec4 normalMetallic = texture(gNormalMetallic, vTexCoord);
    vec4 albedoRoughness = texture(gAlbedoRoughness, vTexCoord);
    float depth = texture(gDepth, vTexCoord).r;
    vec3 viewDir = texture(gViewDir, vTexCoord).rgb;
    float ao = texture(aoMap, vTexCoord).r;

    albedoRoughness.xyz = pow(albedoRoughness.xyz, vec3(2.2));
    
    // Unpack values
    vec3 N = normalize(normalMetallic.rgb * 2.0 - 1.0); // [-1,1] range
    float metallic = normalMetallic.a;
    vec3 albedo = albedoRoughness.rgb;
    float roughness = albedoRoughness.a;
    
    // Reconstruct view direction
    vec3 V = normalize(uCameraPosition - worldPos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < uLightCount; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(uLight[i].position - worldPos);
        vec3 H = normalize(V + L);
        float distance = length(uLight[i].position - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = uLight[i].color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	                
            
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    //vec3 ambient = (kD * diffuse + specular);

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    switch(gDebugMode)
    {
        case 1: // Albedo
            FragColor = vec4(albedo, 1.0);
            return;
        case 2: // Normal
            FragColor = vec4(N, 1.0);
            return;
        case 3: // Position
            FragColor = vec4(worldPos, 1.0);
            return;
        case 4: // View Direction
            FragColor = vec4(viewDir, 1.0);
            return;
        case 5: // Depth
            float linDepth = linearizeDepth(depth, 0.1f, 10.0f);
            FragColor = vec4(vec3(linDepth), 1.0f);
            return;
        case 6: // Metallic
            FragColor = depth == 1.0 ? vec4(0.0) : vec4(vec3(metallic), 1.0);
            return;
        case 7: // Roughness
            FragColor = depth == 1.0 ? vec4(0.0) : vec4(vec3(roughness), 1.0);
            return;
        default: // Final render
            FragColor = depth == 1.0 ? vec4(0.0) : vec4(vec3(ao), 1.0);
            break;
    }
}

#endif
#endif