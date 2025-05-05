///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef DEFERRED_RENDER_QUAD

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
	Light uLight[16];
};

in vec2 vTexCoord;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uPosition;
uniform sampler2D uViewDir;
uniform sampler2D uDepth;

uniform int uDebugMode;

layout(location=0) out vec4 oColor;

vec3 CalcDirLight(Light aLight, vec3 aNormal, vec3 aViewDir)
{
    vec3 lightDir = normalize(-aLight.direction);
    vec3 viewDir = normalize(aViewDir);

    float diff = max(dot(aNormal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, aNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);

    vec3 ambient = aLight.color * 0.2;
    vec3 diffuse = texture(uAlbedo, vTexCoord).xyz * diff;
    vec3 specular = aLight.color * spec * aLight.specularStrength;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(Light aLight, vec3 aNormal, vec3 aPosition, vec3 aViewDir)
{
    vec3 lightDir = normalize(aLight.position - aPosition);
    vec3 viewDir = normalize(aViewDir);

    float diff = max(dot(aNormal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, aNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);

    float distance = length(aLight.position - aPosition);

	float attenuation = 1.0 / (aLight.constant + aLight.linear * distance + aLight.quadratic * (distance * distance)); 

    vec3 ambient = aLight.color * 0.2;
    vec3 diffuse = texture(uAlbedo, vTexCoord).xyz * diff;
    vec3 specular = aLight.color * spec * aLight.specularStrength;

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
	vec3 albedoTex = texture(uAlbedo, vTexCoord).rgb;
    vec3 normalTex = texture(uNormal, vTexCoord).rgb;
    vec3 positionTex = texture(uPosition, vTexCoord).rgb;
    vec3 viewDirTex = texture(uViewDir, vTexCoord).rgb;
    float depthTex = texture(uDepth, vTexCoord).r;

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

    vec3 returnColor = vec3(0.0);
    for(int i = 0; i < uLightCount; ++i)
    {
        vec3 lightResult = vec3(0.0f);

        if(uLight[i].type == 0)
        {
            lightResult += CalcDirLight(uLight[i], normalTex, viewDirTex);
        }
        else if(uLight[i].type == 1)
        {
            lightResult += CalcPointLight(uLight[i], normalTex, positionTex, viewDirTex);
        }

        returnColor += lightResult;
    }

    oColor = depthTex == 1.0 ? vec4(0.0) : vec4(returnColor, 1.0);
}

#endif
#endif