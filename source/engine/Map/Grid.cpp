#include "Grid.hpp"

namespace ge::Map
{

void Grid::create(uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    m_map.clear();

    m_map.resize(m_width * m_height, Grid::Tile::WALL);
}

// Get the Tile at the given position.
Grid::Tile Grid::get(const Position &pos)
{
    if (pos.x < 0 || pos.x > m_width - 1 || pos.y < 0 || pos.y > m_height - 1)
        return Grid::Tile::INVALID;

    return m_map[pos.x + pos.y * m_width];
}

// Set the Tile at the given position.
void Grid::set(const Position &pos, const Grid::Tile tile)
{
    if (pos.x < 0 || pos.x > m_width - 1 || pos.y < 0 || pos.y > m_height - 1)
        return;

    m_map[pos.x + pos.y * m_width] = tile;
}

// returns the Tile data as raw values.
const std::vector<Grid::Tile> &Grid::data()
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
bool Grid::contains(Area area, Tile type)
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
