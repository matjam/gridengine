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

#include "Bounds.hpp"
#include "Position.hpp"
#include <vector>

namespace ge::Map
{

// A Grid based Map using uint8_t as the tile representation.
class Grid
{
  public:
    enum Tile : uint8_t // represents what type of tile a tile is
    { INVALID,          // we return this on out of bounds
      WALL,             // Solid areas of the map
      ROOM,
      HALLWAY,
      DOOR,
      CONNECTOR };

  public:
    void create(uint32_t width, uint32_t height);

    // Get the Tile at the given point.
    virtual Tile get(const Position &pos);

    // Set the Tile at the given point.
    virtual void set(const Position &pos, Tile);

    // returns the Tile data as raw values.
    virtual const std::vector<Tile> &data();

    uint32_t width() const;
    uint32_t height() const;

    // tests if a given area contains any of the given tile
    virtual bool contains(Bounds bounds, Tile type);

  private:
    uint32_t m_width  = 0;
    uint32_t m_height = 0;
    std::vector<Tile> m_map; // actual map data.
};

} // namespace ge::Map
