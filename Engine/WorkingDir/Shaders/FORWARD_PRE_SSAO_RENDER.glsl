///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef FORWARD_RENDER

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

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vViewDir;

layout(binding = 0) uniform sampler2D uAlbedo;

layout(location=0) out vec4 oColor;
layout(location=1) out vec4 oNormal;
layout(location=2) out vec4 oPosition;

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

void main()
{
	vec3 returnColor = vec3(0.0);
    for(int i = 0; i < uLightCount; ++i)
    {
        vec3 lightResult = vec3(0.0f);

        if(uLight[i].type == 0)
        {
            lightResult += CalcDirLight(uLight[i], vNormal, vViewDir);
        }
        else if(uLight[i].type == 1)
        {
            lightResult += CalcPointLight(uLight[i], vNormal, vPosition, vViewDir);
        }

        returnColor += lightResult * uLight[i].color;
    }

    oColor = vec4(returnColor, 1.0);
    oNormal = vec4(vNormal, 1.0f);
    oPosition = vec4(vPosition, 1.0f);
}

#endif
#endif