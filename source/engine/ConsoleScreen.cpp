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

#include <codecvt>
#include <locale>
#include <sstream>
#include <string>

#include "ConsoleScreen.hpp"

#include "FileCache.hpp"

namespace ge
{

ConsoleScreen::ConsoleScreen()
{
}

const void ConsoleScreen::create(uint32_t width, uint32_t height, std::string font_file, uint32_t font_width,
                                 uint32_t font_height)
{
    loadFont(font_file, m_character_height);

    m_width            = width;
    m_height           = height;
    m_character_height = font_height;
    m_character_width  = font_width;

    m_console_bg_vertex_buffer = sf::VertexBuffer(sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Stream);
    m_console_fg_vertex_buffer = sf::VertexBuffer(sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Stream);

    m_console_dirty.resize(m_width * m_height, true);
    m_console_bg.resize(m_width * m_height, 0);
    m_console_fg.resize(m_width * m_height, 2);
    m_console.resize(m_width * m_height, 32);

    m_console_bg_vertices.resize(m_width * m_height * VERTS_PER_CELL, sf::Vertex());
    m_console_fg_vertices.resize(m_width * m_height * VERTS_PER_CELL, sf::Vertex());

    (void)m_console_bg_vertex_buffer.create(m_width * m_height * VERTS_PER_CELL);
    (void)m_console_fg_vertex_buffer.create(m_width * m_height * VERTS_PER_CELL);

    // some default C64 palette colors.
    m_palette_colors = {sf::Color::Black,
                        sf::Color::White,
                        sf::Color(136, 0, 0, 255),
                        sf::Color(170, 255, 238, 255),
                        sf::Color(204, 68, 204, 255),
                        sf::Color(0, 204, 85, 255),
                        sf::Color(0, 0, 170, 255),
                        sf::Color(238, 238, 119, 255),
                        sf::Color(221, 136, 85, 255),
                        sf::Color(102, 68, 0, 255),
                        sf::Color(255, 119, 119, 255),
                        sf::Color(51, 51, 51, 255),
                        sf::Color(119, 119, 119, 255),
                        sf::Color(170, 255, 102, 255),
                        sf::Color(0, 136, 255, 255),
                        sf::Color(187, 187, 187, 255)};

    // set up the vertex buffer locations - we only do this once.

    for (uint32_t y = 0; y < m_height; y++) {
        for (uint32_t x = 0; x < m_width; x++) {
            float v_left   = static_cast<float>(x * m_character_width);
            float v_top    = static_cast<float>(y * m_character_height);
            float v_width  = static_cast<float>(m_character_width);
            float v_height = static_cast<float>(m_character_height);

            setCellPositions(m_console_bg_vertices, sf::Vector2i(x, y), v_left, v_top, v_width, v_height);
            setCellPositions(m_console_fg_vertices, sf::Vector2i(x, y), v_left, v_top, v_width, v_height);
        }
    }
}

const sf::Vector2i ConsoleScreen::characterSize()
{
    return sf::Vector2i(m_character_width, m_character_height);
}

const sf::Vector2i ConsoleScreen::size()
{
    return sf::Vector2i(m_width, m_height);
}

inline void ConsoleScreen::setCellPositions(std::vector<sf::Vertex> &vertices, const sf::Vector2i &location,
                                            float left, float top, float width, float height)
{
    auto offset = (location.x + location.y * m_width) * VERTS_PER_CELL;

    sf::Vector2f tl(left, top);
    sf::Vector2f tr(left + width, top);
    sf::Vector2f br(left + width, top + height);
    sf::Vector2f bl(left, top + height);

    // Triangle 1: top-left, top-right, bottom-right
    vertices[offset + 0].position = tl;
    vertices[offset + 1].position = tr;
    vertices[offset + 2].position = br;
    // Triangle 2: top-left, bottom-right, bottom-left
    vertices[offset + 3].position = tl;
    vertices[offset + 4].position = br;
    vertices[offset + 5].position = bl;
}

inline void ConsoleScreen::setCellColor(std::vector<sf::Vertex> &vertices,
                                        const sf::Vector2i &location, const sf::Color &color)
{
    auto offset = (location.x + location.y * m_width) * VERTS_PER_CELL;

    for (int i = 0; i < VERTS_PER_CELL; i++) {
        vertices[offset + i].color = color;
    }
}

inline void ConsoleScreen::setCellTexCoords(std::vector<sf::Vertex> &vertices,
                                            const sf::Vector2i &location, const sf::Vector2f &texCoords)
{
    auto offset = (location.x + location.y * m_width) * VERTS_PER_CELL;

    sf::Vector2f tl_tex = texCoords;
    sf::Vector2f tr_tex(texCoords.x + m_character_width, texCoords.y);
    sf::Vector2f br_tex(texCoords.x + m_character_width, texCoords.y + m_character_height);
    sf::Vector2f bl_tex(texCoords.x, texCoords.y + m_character_height);

    // Triangle 1
    vertices[offset + 0].texCoords = tl_tex;
    vertices[offset + 1].texCoords = tr_tex;
    vertices[offset + 2].texCoords = br_tex;
    // Triangle 2
    vertices[offset + 3].texCoords = tl_tex;
    vertices[offset + 4].texCoords = br_tex;
    vertices[offset + 5].texCoords = bl_tex;
}

// a vector of sf::Colors to be used as the palette
const void ConsoleScreen::createPalette(const std::vector<sf::Color> &palette_colors)
{
    m_palette_colors = palette_colors;
}

// sets the colors for write/draw operations that don't specify a color.
const void ConsoleScreen::setForeground(const uint32_t color)
{
    m_current_fg = color;
}

const void ConsoleScreen::setBackGround(const uint32_t color)
{
    m_current_bg = color;
}

// writes the given string to a location
const void ConsoleScreen::write(const sf::Vector2i location, const std::string text, const uint32_t max_width,
                                const uint32_t fg, const uint32_t bg)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    auto str = cv.from_bytes(text);

    uint32_t offset = 0;
    for (auto &c : str) {
        if (offset == max_width)
            break;

        if (location.x + offset >= m_width)
            break;

        poke(sf::Vector2i(location.x + offset, location.y), c, fg, bg);
        offset++;
    }
}

const void ConsoleScreen::clear()
{
    rectangle(sf::IntRect(sf::Vector2i{0, 0}, sf::Vector2i{static_cast<int>(m_width), static_cast<int>(m_height)}), 32, 1);
}

const void ConsoleScreen::write(const sf::Vector2i location, const std::string text, const uint32_t max_width,
                                const uint32_t fg)
{
    write(location, text, max_width, fg, m_current_bg);
}

const void ConsoleScreen::write(const sf::Vector2i location, const std::string text, const uint32_t max_width)
{
    write(location, text, max_width, m_current_fg, m_current_bg);
}

const void ConsoleScreen::write(const sf::Vector2i location, const std::string text)
{
    uint32_t max_width = m_width - location.x;
    write(location, text, max_width, m_current_fg, m_current_bg);
}

const void ConsoleScreen::write(const uint32_t x, uint32_t y, const std::string text)
{
    write(sf::Vector2i(x, y), text);
}

const void ConsoleScreen::writeCenter(const sf::IntRect bounds, const std::string text)
{
    auto offset = (bounds.size.x - static_cast<int>(text.length())) / 2;
    write(sf::Vector2i(bounds.position.x + offset, bounds.position.y), text, static_cast<uint32_t>(bounds.size.x));
}

// single character access at a location
inline void ConsoleScreen::poke(const sf::Vector2i location, const char32_t character, const uint32_t fg,
                                const uint32_t bg)
{
    if (location.x > static_cast<int>(m_width) || location.y > static_cast<int>(m_height))
        return;

    const auto offset       = location.x + location.y * m_width;
    m_console[offset]       = character;
    m_console_fg[offset]    = fg;
    m_console_bg[offset]    = bg;
    m_console_dirty[offset] = true;
}

void ConsoleScreen::poke(const uint32_t x, uint32_t y, const char32_t character, const uint32_t fg, const uint32_t bg)
{
    poke(sf::Vector2i(x, y), character, fg, bg);
}

inline const std::tuple<const char32_t, const uint32_t, const uint32_t> ConsoleScreen::peek(const sf::Vector2i location)
{
    return std::make_tuple(m_console[location.x + location.y * m_width],
                           m_console_fg[location.x + location.y * m_width],
                           m_console_bg[location.x + location.y * m_width]);
}

// will draw a box and optionally fill it with a given character using the current bg and fg colors.
const void ConsoleScreen::rectangle(const sf::IntRect bounds, const char32_t character, const bool filled)
{
    if (bounds.size.x == 0 || bounds.size.y == 0) {
        SPDLOG_WARN("given bounds ({},{} {}x{}) that would draw nothing",
                    bounds.position.x,
                    bounds.position.y,
                    bounds.size.x,
                    bounds.size.y);
        return;
    }

    if (bounds.position.x + bounds.size.x > static_cast<int>(m_width) ||
        bounds.position.y + bounds.size.y > static_cast<int>(m_height)) {
        SPDLOG_WARN("given bounds ({},{} {}x{}) that would draw outside of console",
                    bounds.position.x,
                    bounds.position.y,
                    bounds.size.x,
                    bounds.size.y);
        return;
    }

    if (filled) {
        for (int y = bounds.position.y; y < bounds.position.y + bounds.size.y; y++) {
            for (int x = bounds.position.x; x < bounds.position.x + bounds.size.x; x++) {
                if (filled || x == bounds.position.x || x == bounds.position.x + bounds.size.x - 1 ||
                    y == bounds.position.y || y == bounds.position.y + bounds.size.y - 1) {
                    poke(sf::Vector2i(x, y), character, m_current_fg, m_current_bg);
                }
            }
        }
    } else {
        // don't bother scanning the x range for unfilled rectangles.
        for (int x = bounds.position.x; x < bounds.position.x + bounds.size.x; x++) {
            poke(sf::Vector2i(x, bounds.position.y), character, m_current_fg, m_current_bg);
            poke(sf::Vector2i(x, bounds.position.y + bounds.size.y - 1), character, m_current_fg, m_current_bg);
        }
        // skip the top and bottom row. For a height of 1, this means that nothing will
        // be drawn in this loop.
        for (int y = bounds.position.y + 1; y < bounds.position.y + bounds.size.y - 1; y++) {
            poke(sf::Vector2i(bounds.position.x, y), character, m_current_fg, m_current_bg);
            poke(sf::Vector2i(bounds.position.x + bounds.size.x - 1, y), character, m_current_fg, m_current_bg);
        }
    }
}

std::string wrap(const std::string text, size_t line_length = 72)
{
    std::istringstream words(text);
    std::ostringstream wrapped;
    std::string word;

    if (words >> word) {
        wrapped << word;
        size_t space_left = line_length - word.length();
        while (words >> word) {
            if (space_left < word.length() + 1) {
                wrapped << '\n' << word;
                space_left = line_length - word.length();
            } else {
                wrapped << ' ' << word;
                space_left -= word.length() + 1;
            }
        }
    }
    return wrapped.str();
}

const void ConsoleScreen::writeRectangle(const sf::IntRect bounds, const std::string text)
{
    int x = bounds.position.x;
    int y = bounds.position.y;

    auto wrapped = wrap(text, bounds.size.x);
    auto it      = wrapped.begin();

    while (it != wrapped.end()) {
        if (*it == '\n') {
            x = bounds.position.x;
            y++;
            it++;
            continue;
        }

        if (y > bounds.position.y + bounds.size.y - 1)
            return;

        poke(sf::Vector2i(x, y), *it, m_current_fg, m_current_bg);
        x++;
        it++;
    }
}

const void ConsoleScreen::displayCharacterCodes(const sf::Vector2i location, const char32_t start)
{
    uint32_t x = location.x;
    uint32_t y = location.y;

    for (char32_t c = start; x < m_width - 10; c++) {
        poke(sf::Vector2i(x, y), c, 1, 11);
        write(sf::Vector2i(x + 2, y), fmt::format("= {0:x}", static_cast<uint32_t>(c)));
        y++;

        if (y > m_height - 5) {
            x += 10;
            y = location.y;
        }
    }
}

inline bool glyphBit(const FT_GlyphSlot &glyph, const int x, const int y)
{
    int pitch          = abs(glyph->bitmap.pitch);
    unsigned char *row = &glyph->bitmap.buffer[pitch * y];
    char cValue        = row[x >> 3];

    return (cValue & (128 >> (x & 7))) != 0;
}

// When update() is called, we scan through each character and look in the m_glyph_images map
// for the image that holds the glyph. If we don't find one, we construct it from the sf::Font
// that we loaded on start. This is slow for the first time we render that character, but is
// cached after that so should be fast.
//
// These images are then rendered directly to the texture backing the m_console_sprite.
const void ConsoleScreen::update()
{
    std::vector<uint8_t> bitmap;
    bool any_dirty = false;

    const uint32_t total = m_width * m_height;
    const float char_w   = static_cast<float>(m_character_width);
    const float char_h   = static_cast<float>(m_character_height);

    for (uint32_t cell = 0; cell < total; cell++) {
        if (!m_console_dirty[cell])
            continue;

        any_dirty = true;

        const char32_t character = m_console[cell];
        const auto fg_color      = m_palette_colors[m_console_fg[cell]];
        const auto bg_color      = m_palette_colors[m_console_bg[cell]];

        uint32_t atlas_offset = 0;
        auto atlas_offset_it  = m_console_atlas_offset.find(character);

        if (atlas_offset_it == m_console_atlas_offset.end()) {
            // We need to generate a new image for this glyph and write it to the atlas.
            bitmap.resize(m_character_width * m_character_height, 0);

            if (FT_Load_Char(m_face, character, FT_LOAD_RENDER | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO) !=
                FT_Err_Ok) {
                continue;
            }

            FT_GlyphSlot glyph = m_face->glyph;

            for (size_t glyph_y = 0; glyph_y < glyph->bitmap.rows; ++glyph_y) {
                for (size_t glyph_x = 0; glyph_x < glyph->bitmap.width; ++glyph_x) {
                    int imageIndex = (glyph->bitmap.width * glyph_y) + glyph_x;
                    bitmap[imageIndex] = glyphBit(glyph, glyph_x, glyph_y) ? 255 : 0;
                }
            }

            sf::Image glyph_image(sf::Vector2u{m_character_width, m_character_height}, sf::Color::Transparent);

            for (uint32_t offset_y = 0; offset_y < m_character_height; offset_y++) {
                for (uint32_t offset_x = 0; offset_x < m_character_width; offset_x++) {
                    if (bitmap[offset_x + offset_y * m_character_width] == 255) {
                        glyph_image.setPixel(sf::Vector2u{offset_x, offset_y}, sf::Color::White);
                    }
                }
            }

            atlas_offset = setAtlasGlyph(character, glyph_image);
        } else {
            atlas_offset = atlas_offset_it->second;
        }

        // Compute vertex base offset once per cell
        const int base = cell * VERTS_PER_CELL;

        // Set background colors
        for (int i = 0; i < VERTS_PER_CELL; i++)
            m_console_bg_vertices[base + i].color = bg_color;

        // Set foreground colors
        for (int i = 0; i < VERTS_PER_CELL; i++)
            m_console_fg_vertices[base + i].color = fg_color;

        // Set foreground tex coords (inline atlas coord calculation)
        const float tx = static_cast<float>((atlas_offset % m_atlas_width) * m_character_width);
        const float ty = static_cast<float>((atlas_offset / m_atlas_width) * m_character_width);

        // Triangle 1: TL, TR, BR
        m_console_fg_vertices[base + 0].texCoords = {tx, ty};
        m_console_fg_vertices[base + 1].texCoords = {tx + char_w, ty};
        m_console_fg_vertices[base + 2].texCoords = {tx + char_w, ty + char_h};
        // Triangle 2: TL, BR, BL
        m_console_fg_vertices[base + 3].texCoords = {tx, ty};
        m_console_fg_vertices[base + 4].texCoords = {tx + char_w, ty + char_h};
        m_console_fg_vertices[base + 5].texCoords = {tx, ty + char_h};

        m_console_dirty[cell] = false;
    }

    if (any_dirty) {
        (void)m_console_bg_vertex_buffer.update(m_console_bg_vertices.data(), total * VERTS_PER_CELL, 0);
        (void)m_console_fg_vertex_buffer.update(m_console_fg_vertices.data(), total * VERTS_PER_CELL, 0);
    }
}

uint32_t ConsoleScreen::setAtlasGlyph(const char32_t charcode, const sf::Image &image)
{
    // check size of the current atlas and grow it if we need to
    auto atlas_size         = m_console_atlas.getSize();
    uint32_t atlas_capacity = (atlas_size.x / m_character_width) * (atlas_size.y / m_character_height);

    if (m_glyph_count == atlas_capacity) {
        // we are at the maximum for this current size, grow it.
        sf::Texture new_atlas;

        // grow the atlas by 128 characters high every time.
        (void)new_atlas.resize(sf::Vector2u{m_atlas_width * m_character_width,
                                            m_atlas_width * m_character_height + atlas_size.y});
        new_atlas.update(m_console_atlas, sf::Vector2u{0, 0}); // copy the current atlas over
        new_atlas.setSmooth(false);
        m_console_atlas = new_atlas; // replace with the new atlas.

        SPDLOG_INFO("grew the atlas");
    }

    // get the x,y co-ordinates of the image we are adding to the atlas.
    uint32_t x = (m_glyph_count % m_atlas_width) * m_character_width;
    uint32_t y = (m_glyph_count / m_atlas_width) * m_character_width;

    m_console_atlas.update(image, sf::Vector2u{x, y});
    m_console_atlas_offset[charcode] = m_glyph_count;
    auto rv                          = m_glyph_count;
    m_glyph_count++;
    return rv;
}

inline sf::Vector2f ConsoleScreen::getAtlasCoordsForOffset(const uint32_t &offset)
{
    // get the x,y co-ordinates of the image we are adding to the atlas.
    uint32_t x = (offset % m_atlas_width) * m_character_width;
    uint32_t y = (offset / m_atlas_width) * m_character_width;

    return sf::Vector2f(x, y);
}

void ConsoleScreen::loadFont(const std::string font_file, uint32_t pixel_size)
{
    auto font_data = FileCache::Get(font_file);

    auto error = FT_Init_FreeType(&m_library);
    if (error != FT_Err_Ok) {
        SPDLOG_ERROR("unable to initialize freetype");
        return;
    }

    const auto font_data_bytes = (FT_Byte *)font_data->data();

    error = FT_New_Memory_Face(m_library, font_data_bytes, font_data->size(), 0, &m_face);
    if (error == FT_Err_Unknown_File_Format) {
        SPDLOG_ERROR("unable to load unknown font data format");
        return;
    } else if (error != FT_Err_Ok) {
        SPDLOG_ERROR("unable to load font data, unknown error");
        return;
    }

    FT_Set_Pixel_Sizes(m_face,      /* handle to face object */
                       0,           /* pixel_width           */
                       pixel_size); /* pixel_height          */

    SPDLOG_INFO("loaded font {}", m_face->family_name);

    return;
}

void ConsoleScreen::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    target.draw(m_console_bg_vertex_buffer, states);
    states.texture = &m_console_atlas;
    target.draw(m_console_fg_vertex_buffer, states);
}

const void ConsoleScreen::crash()
{
    const auto palette_size = static_cast<uint32_t>(m_palette_colors.size());
    const uint32_t total    = m_width * m_height;

    for (uint32_t cell = 0; cell < total; cell++) {
        m_console[cell]    = 33 + m_rng() % 128;
        m_console_fg[cell] = m_rng() % palette_size;
        m_console_bg[cell] = m_rng() % palette_size;
        m_console_dirty[cell] = true;
    }
}

const void ConsoleScreen::loading()
{
    const auto palette_size = static_cast<uint32_t>(m_palette_colors.size());

    for (uint32_t y = 0; y < m_height; y++) {
        uint32_t bg_color = m_rng() % palette_size;
        for (uint32_t x = 0; x < m_width; x++) {
            const uint32_t cell    = x + y * m_width;
            m_console[cell]        = 32;
            m_console_fg[cell]     = 0;
            m_console_bg[cell]     = bg_color;
            m_console_dirty[cell]  = true;
        }
    }
}
std::vector<sf::Color> ConsoleScreen::palette()
{
    return m_palette_colors;
}

} // namespace ge
