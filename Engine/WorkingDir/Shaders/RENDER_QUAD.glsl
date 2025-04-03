///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef RENDER_QUAD

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

// Debug mode switch (0 = Final Render, 1 = Albedo, 2 = Normal, 3 = Position, 4 = ViewDir)
uniform int uDebugMode;

layout(location=0) out vec4 oColor;

vec3 CalcDirLight(Light aLight, vec3 aNormal, vec3 aViewDir)
{
    vec3 lightDir = normalize(-aLight.direction);

    float diff = max(dot(aNormal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, aNormal);
    float spec = pow(max(dot(aViewDir, reflectDir), 0.0), 2.0);

    vec3 ambient = aLight.color * 0.2;
    vec3 diffuse = texture(uAlbedo, vTexCoord).xyz * diff;
    vec3 specular = aLight.color * spec * 0.1;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(Light aLight, vec3 aNormal, vec3 aPosition, vec3 aViewDir)
{
    vec3 lightDir = normalize(aLight.position - aPosition);

    float diff = max(dot(aNormal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, aNormal);
    float spec = pow(max(dot(aViewDir, reflectDir), 0.0), 2.0);

    float distance = length(aLight.position - aPosition);
    
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));    

    vec3 ambient = aLight.color * 0.2;
    vec3 diffuse = texture(uAlbedo, vTexCoord).xyz * diff;
    vec3 specular = aLight.color * spec * 0.1;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{
	vec3 albedoTex = texture(uAlbedo, vTexCoord).rgb;
    vec3 normalTex = texture(uNormal, vTexCoord).rgb;
    vec3 positionTex = texture(uPosition, vTexCoord).rgb;
    vec3 viewDirTex = texture(uViewDir, vTexCoord).rgb;

    // Debug mode switch
    switch(uDebugMode)
    {
        case 1: // Albedo
            oColor = vec4(albedoTex, 1.0);
            return;
        case 2: // Normal (visualize as RGB)
            oColor = vec4(normalTex, 1.0);
            return;
        case 3: // Position (visualize world space)
            oColor = vec4(positionTex, 1.0);
            return;
        case 4: // View Direction
            oColor = vec4(viewDirTex, 1.0);
            return;
        default: // Final render (default)
            break;
    }

    // Standard deferred shading (final render)
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

        returnColor += lightResult * albedoTex;
    }

    oColor = vec4(returnColor, 1.0); // Multiply with albedo
}

#endif
#endif