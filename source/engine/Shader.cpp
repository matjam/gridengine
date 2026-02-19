/*
 * MIT License
 *
 * Copyright (c) 2020 Nathan Ollerenshaw
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Shader.hpp"

#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>

namespace ge
{

ShaderProgram::~ShaderProgram()
{
    if (m_program)
        glDeleteProgram(m_program);
}

ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept : m_program(other.m_program)
{
    other.m_program = 0;
}

ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other) noexcept
{
    if (this != &other) {
        if (m_program)
            glDeleteProgram(m_program);
        m_program = other.m_program;
        other.m_program = 0;
    }
    return *this;
}

static GLuint compileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        SPDLOG_ERROR("shader compilation failed: {}", info);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool ShaderProgram::compile(const char *vertexSource, const char *fragmentSource)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (!vs) return false;

    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fs) {
        glDeleteShader(vs);
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(m_program, 512, nullptr, info);
        SPDLOG_ERROR("shader link failed: {}", info);
        glDeleteProgram(m_program);
        m_program = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return m_program != 0;
}

void ShaderProgram::use() const
{
    glUseProgram(m_program);
}

void ShaderProgram::setMat4(const char *name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_program, name), 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderProgram::setInt(const char *name, int value) const
{
    glUniform1i(glGetUniformLocation(m_program, name), value);
}

void ShaderProgram::setBool(const char *name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_program, name), static_cast<int>(value));
}

namespace shaders
{

const char *const kVertexSource = R"glsl(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uModel;

out vec4 vColor;
out vec2 vTexCoord;

void main()
{
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
    vTexCoord = aTexCoord;
}
)glsl";

const char *const kFragmentSource = R"glsl(
#version 330 core
in vec4 vColor;
in vec2 vTexCoord;

uniform bool uUseTexture;
uniform sampler2D uAtlas;

out vec4 FragColor;

void main()
{
    if (uUseTexture) {
        FragColor = texture(uAtlas, vTexCoord) * vColor;
    } else {
        FragColor = vColor;
    }
}
)glsl";

} // namespace shaders

} // namespace ge
