#pragma once

#include "openGL_types.inl"

class App;

namespace ShaderLoader 
{
	GLuint CreateProgramFromSource(App* app, String programSource, const char* shaderName);
	u32 LoadProgram(App* app, const char* filepath, const char* programName);
	void ProgramHotReload(App* app);
}