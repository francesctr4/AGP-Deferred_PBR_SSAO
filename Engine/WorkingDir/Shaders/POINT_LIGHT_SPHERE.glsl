///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef POINT_LIGHT_SPHERE

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPos;

uniform mat4 uMVP;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

uniform vec3 uColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(uColor, 1.0);
}

#endif
#endif

// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.