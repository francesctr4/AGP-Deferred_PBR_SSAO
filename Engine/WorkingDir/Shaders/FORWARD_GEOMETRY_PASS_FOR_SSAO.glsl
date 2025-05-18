///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef FORWARD_GEOMETRY_PASS_FOR_SSAO

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;

layout(std140) uniform entityUBO 
{
    mat4 uWorldMatrix;
    mat4 uWorldViewProjectionMatrix;
};

out vec3 vPosition;
out vec3 vNormal;

void main() 
{
    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(uWorldMatrix)));
    vNormal = normalize(normalMatrix * aNormal);
    gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location=0) out vec3 gPosition;
layout(location=1) out vec3 gNormal;

in vec3 vPosition;
in vec3 vNormal;

void main() 
{
    gPosition = vPosition;         // World-space position
    gNormal = normalize(vNormal);  // World-space normal
}

#endif
#endif