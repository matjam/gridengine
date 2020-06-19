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

#include "Position.hpp"
#include <memory>

namespace ge::Map
{

struct Bounds {
    Bounds();
    Bounds(int64_t x1_, int64_t y1_, int64_t x2_, int64_t y2_);
    Bounds(const Bounds &other);
    uint64_t width() const;
    uint64_t height() const;

    // checks if any part of the other Bounds overlap this Bounds
    bool overlaps(const Bounds &other) const;

    // checks if the other bounds is contained within this Bounds
    bool contains(const Bounds &other) const;
    bool contains(const Position &pos) const;

    bool operator<(const Bounds &other) const;

    int64_t x1;
    int64_t y1;
    int64_t x2;
    int64_t y2;
};

} // namespace ge::Map