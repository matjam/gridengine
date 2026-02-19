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

#pragma once

#include <string>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>

namespace ge
{

class ShaderProgram
{
  public:
    ShaderProgram() = default;
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram &) = delete;
    ShaderProgram &operator=(const ShaderProgram &) = delete;
    ShaderProgram(ShaderProgram &&other) noexcept;
    ShaderProgram &operator=(ShaderProgram &&other) noexcept;

    bool compile(const char *vertexSource, const char *fragmentSource);
    void use() const;
    void setMat4(const char *name, const glm::mat4 &mat) const;
    void setInt(const char *name, int value) const;
    void setBool(const char *name, bool value) const;

    GLuint id() const { return m_program; }

  private:
    GLuint m_program = 0;
};

// Embedded GLSL 330 core shaders for the console rendering pipeline
namespace shaders
{
    extern const char *const kVertexSource;
    extern const char *const kFragmentSource;
} // namespace shaders

} // namespace ge
