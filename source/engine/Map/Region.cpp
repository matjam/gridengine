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

namespace ge::Map
{

// Regions are mapped directly to a Tile map so you need to provide a width/height.
void Region::create(uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    m_region_locations.clear();
    m_region_names.clear();

    m_region_names[0] = "DEFAULT";
    m_regions.resize(width * height, 0);
    m_next_region_id = 1;
}

// add a new region and return a reference to the new region Type.
const uint32_t Region::add(std::string name)
{
    m_region_names[m_next_region_id] = name;
    m_next_region_id++;
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

    auto &old_it = m_region_locations.find(old_region);
    if (old_it == m_region_locations.end()) {
        SPDLOG_ERROR("attempt to remove region id {} but it was not found", old_region);
        return;
    }

    auto &new_it = m_region_locations.find(new_region);
    if (new_it == m_region_locations.end()) {
        SPDLOG_ERROR("attempt to use region id {} but it was not found", new_region);
        return;
    }

    for (auto &location : old_it->second) {
        // copy all the locations in the old region to the new region's set.
        new_it->second.insert(location);

        // set the location in the bitmap to the new region.
        m_regions[location.first + location.second * m_width] = new_region;
    }

    // erase our knowledge of the old region.
    m_region_locations.erase(old_region);
    m_region_names.erase(old_region);
}

// return the friendly name for a given region.
const std::string Region::getName(uint32_t region)
{
    auto &region_it = m_region_names.find(region);
    if (region_it == m_region_names.end()) {
        SPDLOG_ERROR("attempt to find region id {} but it was not found", region);
        return;
    }

    return region_it->second;
}

// get a reference to the region id at the given location.
const uint32_t Region::get(uint32_t x, uint32_t y)
{
    return m_regions[x + y * m_width];
}

// set a given location to the region ID
void Region::set(uint32_t x, uint32_t y, uint32_t region)
{
    auto &region_it = m_region_locations.find(region);
    if (region_it == m_region_locations.end()) {
        SPDLOG_ERROR("attempt to use region id {} but it was not found", region);
        return;
    }

    // this region will have been owned by something else.
    auto old_region     = m_regions[x + y * m_width];
    auto &old_region_it = m_region_locations.find(old_region);
    if (old_region_it == m_region_locations.end()) {
        SPDLOG_ERROR("attempt to use region id {} but it was not found", old_region);
        return;
    }

    // remove it from the old region set
    old_region_it->second.erase({x, y});

    // add this x,y to the set of things this region owns.
    region_it->second.insert({x, y});

    // update the bitmap.
    m_regions[x + y * m_width] = region;
}

// get a std::set of all the Locations for a given region ID.
const std::set<Region::Location> &Region::locations(uint32_t region)
{
    auto &region_it = m_region_locations.find(region);
    if (region_it == m_region_locations.end()) {
        SPDLOG_ERROR("attempt to use region id {} but it was not found", region);
        return;
    }

    return region_it->second;
}

} // namespace ge::Map
