///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef LIGHTS

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBitangent;

struct Light 
{
	unsigned int type;
	vec3 color;
	vec3 direction;
	vec3 position;
};

layout(binding = 0, std140) uniform globalUBO
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
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
	vNormal = vec3(uWorldMatrix * vec4(aNormal, 0.0f));
	vTexCoord = aTexCoord;
	vViewDir = uCameraPosition - vPosition;

	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0f);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Light 
{
	unsigned int type;
	vec3 color;
	vec3 direction;
	vec3 position;
};

layout(binding = 0, std140) uniform globalUBO
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(location=0) out vec4 oColor;

vec3 CalcDirLight(Light aLight, vec3 aNormal, vec3 aViewDir)
{
    vec3 lightDir = normalize(-aLight.direction);

    float diff = max(dot(aNormal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, aNormal);
    float spec = pow(max(dot(aViewDir, reflectDir), 0.0), 2.0);

    vec3 ambient = aLight.color * 0.2;
    vec3 diffuse = texture(uTexture, vTexCoord).xyz * diff;
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
    vec3 diffuse = texture(uTexture, vTexCoord).xyz * diff;
    vec3 specular = aLight.color * spec * 0.1;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{
	vec3 returnColor = vec3(0.0);

	for (int i = 0; i < uLightCount; ++i) 
	{
		if(uLight[i].type == 0)
		{
			returnColor += CalcDirLight(uLight[i], vNormal, vViewDir);
		}

		if(uLight[i].type == 1)
		{
			returnColor += CalcPointLight(uLight[i], vNormal, vPosition, vViewDir);
		}
	}

	oColor = vec4(returnColor, 1.0f);
}

#endif
#endif