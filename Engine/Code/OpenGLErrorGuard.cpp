#include "OpenGLErrorGuard.h"

#include <iostream>
#include <format>

OpenGLErrorGuard::OpenGLErrorGuard(const char* message) : msg(message)
{
#ifdef _DEBUG
    CheckGLError("BEGIN", msg);
#endif
}

OpenGLErrorGuard::~OpenGLErrorGuard()
{
#ifdef _DEBUG
    CheckGLError("END", msg);
#endif
}

void OpenGLErrorGuard::CheckGLError(const char* around, const char* message)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << std::format("OpenGL Error [{}]: {} ({}) - {}\n",
            around,
            GetGLErrorString(error),
            std::format("{:#06x}", error),
            message);
    }
}

const char* OpenGLErrorGuard::GetGLErrorString(GLenum error)
{
    switch (error)
    {
        case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default:                   return "UNKNOWN_ERROR";
    }
}
