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

#include <vector>
#include <map>
#include <string>
#include <random>
#include <memory>
#include <mutex>

#include <SFML/Graphics.hpp>

// This all requires a refactor.

namespace ge
{

class Tile
{

  public:
    enum class Type : uint8_t // represents what type of tile a tile is; ooh, strict enum! C++ Advanced Features!
    { INVALID,                // we return this on out of bounds
      WALL,                   // Solid areas of the map
      ROOM,
      HALLWAY,
      DOOR,
      SECRET_DOOR,
      TRAPPED_DOOR,
      STAIRS_UP,
      STAIRS_DOWN,
      TRAP,
      EGRESS,
      CONNECTOR,
    };

    Type type;
    int region_id;

    Tile()
    {
        type      = Type::WALL;
        region_id = -1;
    }

    Tile(Type type, int region_id)
    {
        this->type      = type;
        this->region_id = region_id;
    }
};

class Region // represents a region of tiles; typically rooms, hallways, stairs, etc.
{
  public:
    Region()
    {
        id   = 0;
        name = "unknown_region";
    }

    Region(int id, std::string name)
    {
        this->id   = id;
        this->name = name;
    }

    int id;
    std::string name; // a friendly name for this region.
};

class TileMap
{
  public:
    TileMap(int width, int height);
    void init();

    int w;
    int h;
    int current_region_id;
    bool must_render;

    Region wall_region;
    Tile invalid_wall_tile;
    std::vector<std::vector<Tile>> tiles;
    std::map<int, Region> regions;

    // render the map into a flat uint8_t given pairs of Tile::Type to char32_t.
    // This is intended to give us a way to map the generated map to printable screen characters.
    std::unique_ptr<std::vector<char32_t>> render(std::map<Tile::Type, char32_t> &);

    // lock this when making changes
    std::mutex m_mutex;

    Tile getTile(sf::Vector2i);
    void setTile(sf::Vector2i, Tile::Type, int region_id);

    std::string getRegionName(int);
    int createRegion(std::string); // generate a new region with a given name.
    void updateRegions(int, int);

    bool is(sf::Vector2i, Tile::Type);
    bool isEmpty(sf::Vector2i);
    bool isInRoom(sf::Vector2i);
    bool isCornerInUpLeft(sf::Vector2i);
    bool isCornerInUpRight(sf::Vector2i);
    bool isCornerInDownLeft(sf::Vector2i);
    bool isCornerInDownRight(sf::Vector2i);
    bool isCornerOutDownLeft(sf::Vector2i);
    bool isCornerOutUpLeft(sf::Vector2i);
    bool isCornerOutDownRight(sf::Vector2i);
    bool isCornerOutUpRight(sf::Vector2i);
    bool isWallVerticalUpLeft(sf::Vector2i);
    bool isWallVerticalDownLeft(sf::Vector2i);
    bool isWallVerticalUpRight(sf::Vector2i);
    bool isWallVerticalDownRight(sf::Vector2i);
    bool isWallHorizontalUpLeft(sf::Vector2i);
    bool isWallHorizontalUpRight(sf::Vector2i);
    bool isWallHorizontalDownLeft(sf::Vector2i);
    bool isWallHorizontalDownRight(sf::Vector2i);
};

} // namespace ge
