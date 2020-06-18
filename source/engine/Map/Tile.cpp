#include "Tile.hpp"

namespace ge::Map
{

void Tile::create(uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    m_map.resize(m_width * m_height, Tile::Type::INVALID);
}

// Get the Tile at the given location.
const Tile::Type Tile::get(const uint32_t x, const uint32_t y) const
{
    if (x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1)
        return Tile::Type::INVALID;

    return m_map[x + y * m_width];
}

// Set the Tile at the given location.
void Tile::set(const uint32_t x, const uint32_t y, const Tile::Type tile)
{
    if (x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1)
        return;

    m_map[x + y * m_width] = tile;
}

// returns the Tile data as raw values.
const std::vector<Tile::Type> &Tile::data() const
{
    return m_map;
}

} // namespace ge::Map
