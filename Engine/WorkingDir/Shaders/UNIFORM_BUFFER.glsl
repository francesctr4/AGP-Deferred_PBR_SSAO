///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef UNIFORM_BUFFER

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBitangent;

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0f));
	vNormal = vec3(uWorldMatrix * vec4(aNormal, 0.0f));
	vTexCoord = aTexCoord;

	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0f);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
uniform sampler2D uTexture;

layout(location=0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture, vTexCoord);
}

#endif
#endif