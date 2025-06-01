///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef FORWARD_PRE_SSA0_RENDER

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;

layout(binding = 1, std140) uniform entityUBO
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec3 vPosition;
out vec3 vNormal;

void main()
{
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0f));
	vNormal = mat3(transpose(inverse(uWorldMatrix))) * aNormal;

	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 vPosition;
in vec3 vNormal;

layout(location=0) out vec4 oNormal;
layout(location=1) out vec4 oPosition;

void main()
{
	oNormal = vec4(vNormal, 1.0f);
    oPosition = vec4(vPosition, 1.0f);
}

#endif
#endif