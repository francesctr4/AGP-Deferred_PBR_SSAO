///////////////////////////////////////////////////////////////////////
#ifdef DEFERRED_LIGHT_VOLUME

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;

uniform mat4 uModel;
uniform mat4 uViewProj;

void main()
{
    gl_Position = uViewProj * uModel * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Light {
    int type;
    vec3 color;
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    float radius;
    float specularStrength;
};

uniform Light uLight;
uniform vec3 uCameraPos;

uniform sampler2D uPosition;
uniform sampler2D uNormal;
uniform sampler2D uAlbedo;
uniform sampler2D uViewDir;
uniform sampler2D uDepth;

uniform int uDebugMode;

layout(location=0) out vec4 oColor;

vec3 CalculatePointLight(vec3 position, vec3 normal, vec3 albedo, vec3 viewDir)
{
    vec3 lightDir = normalize(uLight.position - position);
    float distance = length(uLight.position - position);
    
    // Attenuation
    float attenuation = 1.0 / (uLight.constant + uLight.linear * distance + 
                              uLight.quadratic * (distance * distance));
    if(attenuation < 0.001) discard;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * albedo * uLight.color;
    
    // Specular
    vec3 viewDirNorm = normalize(viewDir);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirNorm, reflectDir), 0.0), 2.0);
    vec3 specular = uLight.specularStrength * spec * uLight.color;
    
    // Ambient
    vec3 ambient = uLight.color * 0.2;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

float linearizeDepth(float depth, float near, float far) 
{
    // Convert depth to view-space Z
    float z = depth * 2.0 - 1.0; 
    float viewZ = (2.0 * near * far) / (far + near - z * (far - near));
    
    // Normalize to [0,1] range and invert. White=close, Black=far
    return 1.0 - ((viewZ - near) / (far - near));
}

void main()
{
    vec2 texCoord = gl_FragCoord.xy / textureSize(uPosition, 0);

    vec3 albedoTex = texture(uAlbedo, texCoord).rgb;
    vec3 normalTex = texture(uNormal, texCoord).rgb;
    vec3 positionTex = texture(uPosition, texCoord).rgb;
    vec3 viewDirTex = texture(uViewDir, texCoord).rgb;
    float depthTex = texture(uDepth, texCoord).r;

    switch(uDebugMode)
    {
        case 1: // Albedo
            oColor = vec4(albedoTex, 1.0);
            return;
        case 2: // Normal
            oColor = vec4(normalTex, 1.0);
            return;
        case 3: // Position
            oColor = vec4(positionTex, 1.0);
            return;
        case 4: // View Direction
            oColor = vec4(viewDirTex, 1.0);
            return;
        case 5: // Depth
            float linDepth = linearizeDepth(depthTex, 0.1f, 10.0f);
            oColor = vec4(vec3(linDepth), 1.0f);
            return;
        default: // Final render
            break;
    }

    // Calculate lighting
    vec3 result = CalculatePointLight(positionTex, normalTex, albedoTex, viewDirTex);

    oColor = depthTex == 1.0 ? vec4(0.0) : vec4(result, 1.0);
}

#endif
#endif