#ifndef OPENGL_ERROR_GUARD_H
#define OPENGL_ERROR_GUARD_H

#include "glad/glad.h"

class OpenGLErrorGuard
{
public:

    OpenGLErrorGuard(const char* message);
    ~OpenGLErrorGuard();

private:

    void CheckGLError(const char* around, const char* message);
    const char* GetGLErrorString(GLenum error);

private:

    const char* msg;

};

#endif // OPENGL_ERROR_GUARD_H