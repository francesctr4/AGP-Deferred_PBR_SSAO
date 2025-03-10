#pragma once

static void CheckOpenGLErrors()
{
    GLenum error;
    do
    {
        error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "OpenGL Error: " << std::hex << error << std::dec << std::endl;
        }
    } while (error != GL_NO_ERROR && error != GL_CONTEXT_LOST);
}