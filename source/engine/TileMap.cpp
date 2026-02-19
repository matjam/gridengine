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

#include "TileMap.hpp"
#include "Logging.hpp"

namespace ge
{

std::string TileMap::getRegionName(int region_id)
{
    if (regions.find(region_id) == regions.end()) {
        return "unknown_region_get";
    }
    return regions[region_id].name;
}

TileMap::TileMap(int width, int height)
{
    const std::lock_guard<std::mutex> lock(this->m_mutex);
    w                 = width;
    h                 = height;
    current_region_id = 0;

    wall_region = Region(0, "wall");
    regions[0]  = Region(0, "wall");

    invalid_wall_tile = Tile(Tile::Type::INVALID, wall_region.id);

    for (auto y = 0; y < h; y++) {
        tiles.push_back(std::vector<Tile>());
        for (auto x = 0; x < w; x++) {
            tiles.back().push_back(Tile(Tile::Type::WALL, wall_region.id));
        }
    }

    SPDLOG_INFO("TileMap initialized with size {}x{}", width, height);
}

Tile TileMap::getTile(Vec2i location)
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    if (location.x < 0 || location.x > w - 1 || location.y < 0 || location.y > h - 1) {
        return invalid_wall_tile;
    }

    auto tile = tiles[location.y][location.x];

    return tile;
}

std::unique_ptr<std::vector<char32_t>> TileMap::render(std::map<Tile::Type, char32_t> &mapping)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    auto mapped_tiles = std::make_unique<std::vector<char32_t>>(w * h, mapping[Tile::Type::INVALID]);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            auto tile                  = tiles[y][x];
            auto mapped_tile           = mapping[tile.type];
            (*mapped_tiles)[x + y * w] = mapped_tile;
        }
    }

    return mapped_tiles;
}

void TileMap::setTile(Vec2i location, Tile::Type type, int region_id)
{
    assert(region_id != -1);

    if (location.x < 0 || location.x > w - 1 || location.y < 0 || location.y > h - 1)
        return;

    const std::lock_guard<std::mutex> lock(m_mutex);
    if (regions.find(region_id) == regions.end()) {
        SPDLOG_ERROR("TileMap::setTile for {},{} failed as region does not exist", location.x, location.y);
        return;
    }

    auto tile = tiles[location.y][location.x];

    // set the tile to the new data.
    tile.region_id                = region_id;
    tile.type                     = type;
    tiles[location.y][location.x] = tile;

    must_render = true;
}

int TileMap::createRegion(std::string name)
{
    current_region_id++;
    auto region_name = fmt::format("{}/{}", name, current_region_id);

    regions[current_region_id] = Region(current_region_id, region_name);

    return current_region_id;
}

void TileMap::updateRegions(int old_region_id, int new_region_id)
{
    int tile_count = 0;
    const std::lock_guard<std::mutex> lock(m_mutex);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (tiles[y][x].region_id == old_region_id) {
                tiles[y][x].region_id = new_region_id;
                tile_count++;
            }
        }
    }
}

bool TileMap::is(Vec2i loc, Tile::Type type)
{
    if (loc.x < 0 || loc.y < 0 || loc.x > w - 1 || loc.y > h - 1) {
        if (type == Tile::Type::WALL || type == Tile::Type::INVALID) {
            return true;
        } else {
            return false;
        }
    }

    return tiles[loc.y][loc.x].type == type;
}

bool TileMap::isEmpty(Vec2i loc)
{
    return is(loc, Tile::Type::WALL) || is(loc, Tile::Type::INVALID);
}

bool TileMap::isInRoom(Vec2i loc)
{
    return (!isEmpty(loc) && !isEmpty(Vec2i{loc.x - 1, loc.y}) && !isEmpty(Vec2i{loc.x, loc.y - 1}) &&
            !isEmpty(Vec2i{loc.x + 1, loc.y}) && !isEmpty(Vec2i{loc.x, loc.y + 1}));
}

bool TileMap::isCornerInUpLeft(Vec2i loc)
{
    return (isEmpty(loc) &&                                // origin
            isEmpty(Vec2i{loc.x - 1, loc.y}) &&     // left
            isEmpty(Vec2i{loc.x, loc.y - 1}) &&     // down
            !isEmpty(Vec2i{loc.x - 1, loc.y - 1})); // down left
}

bool TileMap::isCornerInUpRight(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x + 1, loc.y}) &&     // right
            isEmpty(Vec2i{loc.x, loc.y - 1}) &&     // down
            !isEmpty(Vec2i{loc.x + 1, loc.y - 1})); // down right
}

bool TileMap::isCornerInDownLeft(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x - 1, loc.y}) &&     // left
            isEmpty(Vec2i{loc.x, loc.y + 1}) &&     // down
            !isEmpty(Vec2i{loc.x - 1, loc.y + 1})); // down left
}

bool TileMap::isCornerInDownRight(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x + 1, loc.y}) &&     // right
            isEmpty(Vec2i{loc.x, loc.y + 1}) &&     // down
            !isEmpty(Vec2i{loc.x + 1, loc.y + 1})); // down right
}

bool TileMap::isCornerOutDownLeft(Vec2i loc)
{
    return (isEmpty(loc) &&                             //
            !isEmpty(Vec2i{loc.x - 1, loc.y}) && // left
            !isEmpty(Vec2i{loc.x, loc.y + 1}));  // down
}

bool TileMap::isCornerOutUpLeft(Vec2i loc)
{
    return (isEmpty(loc) &&                             //
            !isEmpty(Vec2i{loc.x, loc.y - 1}) && // up
            !isEmpty(Vec2i{loc.x - 1, loc.y}));  // left
}

bool TileMap::isCornerOutDownRight(Vec2i loc)
{
    return (isEmpty(loc) &&                             //
            !isEmpty(Vec2i{loc.x + 1, loc.y}) && // right
            !isEmpty(Vec2i{loc.x, loc.y + 1}));  // down
}

bool TileMap::isCornerOutUpRight(Vec2i loc)
{
    return (isEmpty(loc) &&                             //
            !isEmpty(Vec2i{loc.x + 1, loc.y}) && // right
            !isEmpty(Vec2i{loc.x, loc.y - 1}));  // up
}

bool TileMap::isWallVerticalUpLeft(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x, loc.y - 1}) &&     // up
            !isEmpty(Vec2i{loc.x - 1, loc.y}) &&    // left
            !isEmpty(Vec2i{loc.x - 1, loc.y - 1})); // up left
}

bool TileMap::isWallVerticalDownLeft(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x, loc.y + 1}) &&     // down
            !isEmpty(Vec2i{loc.x - 1, loc.y}) &&    // left
            !isEmpty(Vec2i{loc.x - 1, loc.y + 1})); // down left
}

bool TileMap::isWallVerticalUpRight(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x, loc.y - 1}) &&     // up
            !isEmpty(Vec2i{loc.x + 1, loc.y}) &&    // right
            !isEmpty(Vec2i{loc.x + 1, loc.y - 1})); // up right
}

bool TileMap::isWallVerticalDownRight(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x, loc.y + 1}) &&     // down
            !isEmpty(Vec2i{loc.x + 1, loc.y}) &&    // right
            !isEmpty(Vec2i{loc.x + 1, loc.y + 1})); // down right
}

bool TileMap::isWallHorizontalUpLeft(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x - 1, loc.y}) &&     // left
            !isEmpty(Vec2i{loc.x, loc.y - 1}) &&    // up
            !isEmpty(Vec2i{loc.x - 1, loc.y - 1})); // up left
}

bool TileMap::isWallHorizontalUpRight(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x + 1, loc.y}) &&     // right
            !isEmpty(Vec2i{loc.x, loc.y - 1}) &&    // up
            !isEmpty(Vec2i{loc.x + 1, loc.y - 1})); // up right
}

bool TileMap::isWallHorizontalDownLeft(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x - 1, loc.y}) &&     // left
            !isEmpty(Vec2i{loc.x, loc.y + 1}) &&    // down
            !isEmpty(Vec2i{loc.x - 1, loc.y + 1})); // down left
}

bool TileMap::isWallHorizontalDownRight(Vec2i loc)
{
    return (isEmpty(loc) &&                                //
            isEmpty(Vec2i{loc.x + 1, loc.y}) &&     // right
            !isEmpty(Vec2i{loc.x, loc.y + 1}) &&    // down
            !isEmpty(Vec2i{loc.x + 1, loc.y + 1})); // down right
}

} // namespace ge
