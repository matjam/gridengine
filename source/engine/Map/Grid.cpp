#include "Grid.hpp"

namespace ge::Map
{

void Grid::create(uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    m_map.clear();

    m_map.resize(m_width * m_height, Grid::Type::WALL);
}

// Get the Tile at the given point.
const Grid::Type Grid::get(const Point point)
{
    if (point.x > m_width - 1 || point.y > m_height - 1)
        return Grid::Type::INVALID;

    return m_map[point.x + point.y * m_width];
}

// Set the Tile at the given point.
void Grid::set(const Point point, const Grid::Type tile)
{
    if (point.x > m_width - 1 || point.y > m_height - 1)
        return;

    m_map[point.x + point.y * m_width] = tile;
}

// returns the Tile data as raw values.
const std::vector<Grid::Type> &Grid::data()
{
    return m_map;
}

uint32_t Grid::width() const
{
    return m_width;
}

uint32_t Grid::height() const
{
    return m_height;
}

// scans every tile in the given area.
bool Grid::contains(Area area, Type type)
{
    for (auto y = area.top; y < area.top + area.height; y++) {
        for (auto x = area.left; x < area.left + area.width; x++) {
            if (x > m_width - 1 || y > m_height - 1)
                continue;

            if (m_map[x + y * m_width] == type)
                return true;
        }
    }

    return false;
}

} // namespace ge::Map
