#include "ShaderLoader.h"

#include "Core/engine.h"

#include <format>

GLuint ShaderLoader::CreateProgramFromSource(App* app, String programSource, const char* shaderName)
{
    app->shaderErrors.clear();

    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);

        app->shaderErrors.push_back(std::format(
            "glCompileShader() failed with vertex shader '{}'\nReported message:\n{}\n",
            shaderName, infoLogBuffer));

        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);

        app->shaderErrors.push_back(std::format(
            "glCompileShader() failed with fragment shader '{}'\nReported message:\n{}\n",
            shaderName, infoLogBuffer));

        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);

        app->shaderErrors.push_back(std::format(
            "glLinkProgram() failed with program '{}'\nReported message:\n{}\n",
            shaderName, infoLogBuffer));

        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 ShaderLoader::LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(app, programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

    if (program.handle != 0)
    {
        GLint attributeCount = 0UL;
        glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

        for (size_t i = 0; i < attributeCount; ++i)
        {
            GLchar attributeName[248];
            GLsizei attributeNameLength = 0UL;
            GLsizei attributeSize = 0UL;
            GLenum attributeType = 0UL;

            glGetActiveAttrib(program.handle, i, ARRAY_COUNT(attributeName),
                &attributeNameLength, &attributeSize, &attributeType, attributeName);

            GLuint attibuteLocation = glGetAttribLocation(program.handle, attributeName);

            program.vertexInputLayout.attributes.push_back(
                VertexShaderAttribute(static_cast<u8>(attibuteLocation), static_cast<u8>(attributeSize)));
        }
    }

    app->programs.push_back(program);

    return app->programs.size() - 1;
}

void ShaderLoader::ProgramHotReload(App* app)
{
    for (u64 i = 0; i < app->programs.size(); ++i)
    {
        Program& program = app->programs[i];
        u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimestamp > program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            String programSource = ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = CreateProgramFromSource(app, programSource, programName);
            program.lastWriteTimestamp = currentTimestamp;
        }
    }
}
