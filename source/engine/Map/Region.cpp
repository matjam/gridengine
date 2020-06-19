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

#include "Region.hpp"
#include "spdlog/spdlog.h"
#include <exception>
#include <utility>

namespace ge::Map
{

Region::Region(const Region &other)
{
    m_next_region_id = other.m_next_region_id;
    m_width          = other.m_width;
    m_height         = other.m_height;

    m_regions          = other.m_regions;
    m_region_positions = other.m_region_positions;
    m_region_names     = other.m_region_names;
}

// Regions are mapped directly to a Tile map so you need to provide a width/height.
void Region::create(uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    m_regions.clear();
    m_region_positions.clear();
    m_region_names.clear();

    m_region_names[0] = "DEFAULT";
    m_region_positions[0].clear();
    m_regions.resize(width * height, 0);
    m_next_region_id = 1;
}

// add a new region and return a reference to the new region Type.
uint32_t Region::add(std::string name)
{
    auto this_region_id = m_next_region_id;

    m_region_names[this_region_id] = std::move(name);
    m_region_positions[this_region_id].clear();
    m_next_region_id++;
    return this_region_id;
}

// remove a region and set all of it's tiles to the given region.
void Region::remove(uint32_t old_region, uint32_t new_region)
{
    if (old_region == new_region) {
        SPDLOG_WARN("attempt to remove region id {} name {} and replace it with itself",
                    new_region,
                    m_region_names[new_region]);
        return;
    }

    auto old_it = m_region_positions.find(old_region);
    if (old_it == m_region_positions.end()) {
        throw std::runtime_error(fmt::format("attempt to use region id {} but it was not found", old_region));
    }

    auto new_it = m_region_positions.find(new_region);
    if (new_it == m_region_positions.end()) {
        throw std::runtime_error(fmt::format("attempt to use region id {} but it was not found", new_region));
    }

    for (auto &point : old_it->second) {
        // copy all the points in the old region to the new region's set.
        new_it->second.insert(point);

        // set the point in the bitmap to the new region.
        m_regions[point.x + point.y * m_width] = new_region;
    }

    // erase our knowledge of the old region.
    m_region_positions.erase(old_region);
    m_region_names.erase(old_region);
}

// return the friendly name for a given region.
std::string Region::getName(uint32_t region)
{
    auto region_it = m_region_names.find(region);
    if (region_it == m_region_names.end()) {
        throw std::runtime_error(fmt::format("attempt to find region id {} but it was not found", region));
    }

    return region_it->second;
}

// get a reference to the region id at the given point.
uint32_t Region::get(const Position &point)
{
    return m_regions[point.x + point.y * m_width];
}

// set a given point to the region ID
void Region::set(const Position &point, uint32_t region)
{
    if (point.x > m_width - 1 || point.y > m_height - 1)
        return;

    auto region_it = m_region_positions.find(region);
    if (region_it == m_region_positions.end()) {
        throw std::runtime_error(fmt::format("attempt to use region id {} but it was not found", region));
    }

    // this region will have been owned by something else.
    auto old_region    = m_regions[point.x + point.y * m_width];
    auto old_region_it = m_region_positions.find(old_region);
    if (old_region_it == m_region_positions.end()) {
        throw std::runtime_error(fmt::format("attempt to use old region id {} but it was not found", old_region));
    }

    // remove it from the old region set
    old_region_it->second.erase(point);

    // add this x,y to the set of things this region owns.
    region_it->second.insert(point);

    // update the bitmap.
    m_regions[point.x + point.y * m_width] = region;
}

// get a std::set of all the Points for a given region ID.
std::vector<Position> Region::positions(uint32_t region)
{
    std::vector<Position> p;
    p.resize(0);

    auto region_it = m_region_positions.find(region);
    if (region_it == m_region_positions.end()) {
        throw std::runtime_error(fmt::format("attempt to use region id {} but it was not found", region));
    }

    for (auto point : region_it->second) {
        p.push_back(point);
    }

    return p;
}

std::vector<uint32_t> Region::regions()
{
    return m_regions;
}

} // namespace ge::Map
