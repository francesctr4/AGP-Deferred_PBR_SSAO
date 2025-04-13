#pragma once

#include "openGL_types.inl"

class App;

namespace ImageLoader 
{
	Image LoadImage(const char* filename);
	void FreeImage(Image image);

	GLuint CreateTexture2DFromImage(Image image);
	u32 LoadTexture2D(App* app, const char* filepath);
}
