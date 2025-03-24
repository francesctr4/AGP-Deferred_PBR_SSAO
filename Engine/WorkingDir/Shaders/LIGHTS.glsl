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

void main()
{
	vec3 lightDir = normalize(-uLight[0].direction);
	vec3 reflectDir = reflect(-lightDir, vNormal);

    float diff = max(dot(vNormal, lightDir), 0.0);
    float spec = pow(max(dot(vViewDir, reflectDir), 0.0), 0.2f);
	vec4 finalColor = texture(uTexture, vTexCoord) * diff * spec;
	oColor = finalColor * vec4(uLight[0].color, 1.0f);
}

#endif
#endif