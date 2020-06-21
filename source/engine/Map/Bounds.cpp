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

#include "Bounds.hpp"
#include <spdlog/spdlog.h>
#include <tuple>

namespace ge::Map
{

Bounds::Bounds()
{
    m_left   = 0;
    m_top    = 0;
    m_width  = 0;
    m_height = 0;
}

Bounds::Bounds(int64_t left, int64_t top, uint64_t width, uint64_t height)
{
    m_left   = left;
    m_top    = top;
    m_width  = width;
    m_height = height;
}

Bounds::Bounds(const Bounds &other)
{
    m_left   = other.m_left;
    m_top    = other.m_top;
    m_width  = other.m_width;
    m_height = other.m_height;
}

bool Bounds::operator<(const Bounds &other) const
{
    return std::tie(m_left, m_top, m_width, m_height) <
           std::tie(other.m_left, other.m_top, other.m_width, other.m_height);
}

bool Bounds::contains(const Position &pos) const
{
    int64_t right      = m_left + m_width;
    int64_t bottom     = m_top + m_height;
    bool in_horizontal = m_left <= pos.x && pos.x <= right;
    bool in_vertical   = m_top <= pos.y && pos.y <= bottom;
    bool in_bounds     = in_horizontal && in_vertical;

    return in_bounds;
}

uint64_t Bounds::width() const
{
    return m_width;
}

uint64_t Bounds::height() const
{
    return m_height;
}
int64_t Bounds::left() const
{
    return m_left;
}
int64_t Bounds::top() const
{
    return m_top;
}

} // namespace ge::Map