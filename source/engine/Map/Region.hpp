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
#include <utility>
#include <string>
#include <map>
#include <set>
#include <mutex>

#include "Grid.hpp"

/**
 * a Region is used to allow the game to define different areas of the map that
 * have different properties. For example, you might have a Region that is high
 * security, and the player entering that region would be instantly attacked.
 *
 * Regions are used in a few ways;
 *
 *   1. can create a new region and give it a name.
 *
 *   2. You can add points to a region.
 *
 *   3. You can move points from one region to another.
 *
 *   4. You can delete a region, moving all it's points into another region.
 *
 *   5. You can look up a Region and get a list of all the points in that
 *      region.
 *
 *   6. You can look up a Tile and get the Region that it belongs to.
 *
 * Regions are limited to a uint32_t identifier; but when you create them you
 * will also be able to set a friendly label for the region. I fyou have more
 * than 2^32 regions, then I think you have other issues.
 *
 * By default, there is a "DEFAULT" region, which owns all points.
 *
 * It's intended that at map generation time you would maintain several Region
 * instances to keep track of what rooms you generate, maybe what seed you
 * generate for each room, so you can do things like differentiate the rooms in
 * the way you see fit.
 *
 * Regions are also helpful when you're doing map generation, for some
 * algorithms.
 **/

namespace ge::Map
{

class Region
{
  public:
    Region() = default;
    Region(const Region &other);

    // Regions are mapped directly to a Tile map so you need to provide a width/height.
    void create(uint32_t width, uint32_t height);

    // add a new region and return a reference to the new region id.
    const uint32_t add(std::string);

    // remove a region and set all of it's tiles to the given region.
    void remove(uint32_t old_region, uint32_t new_region);

    // return the friendly name for a given region.
    const std::string getName(uint32_t region);

    // get the region ID at the given location.
    const uint32_t get(Grid::Point point);

    // set a given location to the region ID. This will remove it from it's old region
    // and add it to the new region.
    void set(Grid::Point point, uint32_t region);

    // get a std::set of all the Points for a given region ID. This returns a copy,
    // so use with care.
    const std::vector<Grid::Point> points(uint32_t);

    // get a vector of all region IDs
    const std::vector<uint32_t> regions();

  private:
    uint32_t m_next_region_id;
    uint32_t m_width;
    uint32_t m_height;

    // a 2d vector of all the points and their region IDs.
    std::vector<uint32_t> m_regions;

    // a map to a set of points that a given region ID owns.
    std::map<uint32_t, std::set<Grid::Point>> m_region_points;

    // a map of region IDs to friendly names.
    std::map<uint32_t, std::string> m_region_names;
};

}; // namespace ge::Map
