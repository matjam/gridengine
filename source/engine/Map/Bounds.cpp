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
namespace ge::Map
{

Bounds::Bounds()
{
    x1 = 0;
    y1 = 0;
    x2 = 0;
    y2 = 0;
}

Bounds::Bounds(int64_t x1_, int64_t y1_, int64_t x2_, int64_t y2_)
{
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;
}

Bounds::Bounds(const Bounds &other)
{
    x1 = other.x1;
    y1 = other.y1;
    x2 = other.x2;
    y2 = other.y2;
}

bool Bounds::operator<(const Bounds &other) const
{
    return x1 * y1 * x2 * y2 < other.x1 * other.y1 + other.x2 * other.y2;
}

bool Bounds::overlaps(const Bounds &other) const
{
    return !(x1 > other.x2 || x2 < other.x1 || y1 > other.y2 || y2 < other.y1);
}

bool Bounds::contains(const Bounds &other) const
{
    return (x1 < other.x1 && x2 > other.x2 && y1 < other.y1 && y2 > other.y2);
}

bool Bounds::contains(const Position &pos) const
{
    return (x1 < pos.x && pos.x < x2 && y1 < pos.y && pos.y < x2);
}

uint64_t Bounds::width() const
{
    return x2 - x1;
}

uint64_t Bounds::height() const
{
    return y2 - y1;
}

} // namespace ge::Map