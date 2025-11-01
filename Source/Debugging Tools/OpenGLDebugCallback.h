#pragma once

#ifdef _DEBUG
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    // Ignoring the following WARNING: Buffer detailed info: Buffer objects will use VIDEO memory as the source for buffer object operations.
    if (id == 131185) return;

    std::string logMessage = "[ERROR] OPENGL CALLBACK: ";

    switch (type) {

    case GL_DEBUG_TYPE_ERROR:
        logMessage += "Type: ERROR, ";
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        logMessage += "Type: DEPRECATED_BEHAVIOR, ";
        break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        logMessage += "Type: UNDEFINED_BEHAVIOR, ";
        break;

    case GL_DEBUG_TYPE_PORTABILITY:
        logMessage += "Type: PORTABILITY, ";
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
        logMessage += "Type: PERFORMANCE, ";
        break;

    case GL_DEBUG_TYPE_OTHER:
        logMessage += "Type: OTHER, ";
        break;

    }

    switch (severity) {

    case GL_DEBUG_SEVERITY_LOW:
        logMessage += "Severity: LOW, ";
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        logMessage += "Severity: MEDIUM, ";
        break;

    case GL_DEBUG_SEVERITY_HIGH:
        logMessage += "Severity: HIGH, ";
        break;

    }

    logMessage += "ID: " + std::to_string(id) + ", ";

    logMessage += "MESSAGE: " + std::string(("%s", message));

    logMessage += "\n";

    std::cout << logMessage << std::endl;
}

void EnableOpenGLDebugCallback() 
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
}
#endif
