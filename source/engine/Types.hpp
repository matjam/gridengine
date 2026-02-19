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

#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace ge
{

using Vec2f = glm::vec2;
using Vec2i = glm::ivec2;
using Vec2u = glm::uvec2;

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    static const Color Black;
    static const Color White;
    static const Color Transparent;

    bool operator==(const Color &other) const = default;
};

struct IntRect {
    Vec2i position{0, 0};
    Vec2i size{0, 0};

    constexpr IntRect() = default;
    constexpr IntRect(Vec2i pos, Vec2i sz) : position(pos), size(sz) {}
};

struct FloatRect {
    Vec2f position{0.f, 0.f};
    Vec2f size{0.f, 0.f};

    constexpr FloatRect() = default;
    constexpr FloatRect(Vec2f pos, Vec2f sz) : position(pos), size(sz) {}
};

} // namespace ge
