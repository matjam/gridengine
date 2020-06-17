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
#include <random>
#include <sstream>
#include <string>

#include "ConsoleScreen.hpp"

#include "FileCache.hpp"

namespace Engine
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

    auto texture_width  = width * m_character_width;
    auto texture_height = height * m_character_height;

    m_console_bg_vertex_buffer = sf::VertexBuffer(sf::Quads, sf::VertexBuffer::Usage::Stream);
    m_console_fg_vertex_buffer = sf::VertexBuffer(sf::Quads, sf::VertexBuffer::Usage::Stream);

    m_console_dirty.resize(m_width * m_height, true);
    m_console_bg.resize(m_width * m_height, 0);
    m_console_fg.resize(m_width * m_height, 2);
    m_console.resize(m_width * m_height, 32);

    m_console_bg_vertices.resize(m_width * m_height * 4, sf::Vertex());
    m_console_fg_vertices.resize(m_width * m_height * 4, sf::Vertex());

    m_console_bg_vertex_buffer.create(m_width * m_height * 4);
    m_console_fg_vertex_buffer.create(m_width * m_height * 4);

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
            float v_left   = x * m_character_width;
            float v_top    = y * m_character_height;
            float v_width  = m_character_width;
            float v_height = m_character_height;

            auto q = getQuadForScreenLocation(m_console_bg_vertices, sf::Vector2i(x, y));

            q.a = sf::Vector2f(v_left, v_top);
            q.b = sf::Vector2f(v_left + v_width, v_top);
            q.c = sf::Vector2f(v_left + v_width, v_top + v_height);
            q.d = sf::Vector2f(v_left, v_top + v_height);

            auto r = getQuadForScreenLocation(m_console_fg_vertices, sf::Vector2i(x, y));

            r.a = sf::Vector2f(v_left, v_top);
            r.b = sf::Vector2f(v_left + v_width, v_top);
            r.c = sf::Vector2f(v_left + v_width, v_top + v_height);
            r.d = sf::Vector2f(v_left, v_top + v_height);
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

inline struct ConsoleScreen::Quad ConsoleScreen::getQuadForScreenLocation(std::vector<sf::Vertex> &vertices,
                                                                          const sf::Vector2i &location)
{
    auto offset = (location.x + location.y * m_width) * 4;

    return ConsoleScreen::Quad{vertices[offset], vertices[offset + 1], vertices[offset + 2], vertices[offset + 3]};
}

inline void ConsoleScreen::setQuadColorForScreenLocation(std::vector<sf::Vertex> &vertices,
                                                         const sf::Vector2i &location, const sf::Color &color)
{
    auto q = getQuadForScreenLocation(vertices, location);

    q.a.color = color;
    q.b.color = color;
    q.c.color = color;
    q.d.color = color;
}

inline void ConsoleScreen::setTexCoordsForScreenLocation(std::vector<sf::Vertex> &vertices,
                                                         const sf::Vector2i &location, const sf::Vector2f &texCoords)
{
    auto q        = getQuadForScreenLocation(vertices, location);
    q.a.texCoords = texCoords;
    q.b.texCoords = sf::Vector2f(texCoords.x + m_character_width, texCoords.y);
    q.c.texCoords = sf::Vector2f(texCoords.x + m_character_width, texCoords.y + m_character_height);
    q.d.texCoords = sf::Vector2f(texCoords.x, texCoords.y + m_character_height);
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
        if (offset == max_width) {
            break;
        }

        poke(sf::Vector2i(location.x + offset, location.y), c, fg, bg);
        offset++;
    }
}

const void ConsoleScreen::clear()
{
    rectangle(sf::IntRect(0, 0, m_width, m_height), 32, 1);
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

const void ConsoleScreen::writeCenter(const sf::IntRect bounds, const std::string text)
{
    auto offset = (bounds.width - text.length()) / 2;
    write(sf::Vector2i(bounds.left + offset, bounds.top), text, bounds.width);
}

// single character access at a location
inline void ConsoleScreen::poke(const sf::Vector2i location, const char32_t character, const uint32_t fg,
                                const uint32_t bg)
{
    const auto offset       = location.x + location.y * m_width;
    m_console[offset]       = character;
    m_console_fg[offset]    = fg;
    m_console_bg[offset]    = bg;
    m_console_dirty[offset] = true;
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
    if (bounds.width == 0 || bounds.height == 0) {
        SPDLOG_WARN("given bounds ({},{} {}x{}) that would draw nothing",
                    bounds.left,
                    bounds.top,
                    bounds.width,
                    bounds.height);
        return;
    }

    if (bounds.left + bounds.width > m_width || bounds.top + bounds.height > m_height) {
        SPDLOG_WARN("given bounds ({},{} {}x{}) that would draw outside of console",
                    bounds.left,
                    bounds.top,
                    bounds.width,
                    bounds.height);
        return;
    }

    if (filled) {
        for (uint32_t y = bounds.top; y < bounds.top + bounds.height; y++) {
            for (uint32_t x = bounds.left; x < bounds.left + bounds.width; x++) {
                if (filled || x == bounds.left || x == bounds.left + bounds.width - 1 || y == bounds.top ||
                    y == bounds.top + bounds.height - 1) {
                    poke(sf::Vector2i(x, y), character, m_current_fg, m_current_bg);
                }
            }
        }
    } else {
        // don't bother scanning the x range for unfilled rectangles.
        for (uint32_t x = bounds.left; x < bounds.left + bounds.width; x++) {
            poke(sf::Vector2i(x, bounds.top), character, m_current_fg, m_current_bg);
            poke(sf::Vector2i(x, bounds.top + bounds.height - 1), character, m_current_fg, m_current_bg);
        }
        // skip the top and bottom row. For a height of 1, this means that nothing will
        // be drawn in this loop.
        for (uint32_t y = bounds.top + 1; y < bounds.top + bounds.height - 1; y++) {
            poke(sf::Vector2i(bounds.left, y), character, m_current_fg, m_current_bg);
            poke(sf::Vector2i(bounds.left + bounds.width - 1, y), character, m_current_fg, m_current_bg);
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
    uint32_t x = bounds.left;
    uint32_t y = bounds.top;

    auto wrapped = wrap(text, bounds.width);
    auto it      = wrapped.begin();

    while (it != wrapped.end()) {
        if (*it == '\n') {
            x = bounds.left;
            y++;
            it++;
            continue;
        }

        if (y > bounds.top + bounds.height - 1)
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
    sf::Clock timer;
    std::vector<uint8_t> bitmap;

    for (uint32_t y = 0; y < m_height; y++) {
        for (uint32_t x = 0; x < m_width; x++) {

            uint32_t atlas_offset = 0;

            auto &[character, fg, bg] = peek(sf::Vector2i(x, y));
            auto fg_color             = m_palette_colors[fg];
            auto bg_color             = m_palette_colors[bg];

            auto atlas_offset_it = m_console_atlas_offset.find(character);

            if (atlas_offset_it == m_console_atlas_offset.end()) {
                // We need to generate a new image for this glyph and write it to the atlas.

                // I think this is larger than what we need
                bitmap.resize(m_character_width * m_character_height, 0);

                if (FT_Load_Char(m_face, character, FT_LOAD_RENDER | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO) !=
                    FT_Err_Ok) {
                    continue;
                }

                FT_GlyphSlot glyph = m_face->glyph;
                // the glyphs coming out of FreeType are bitmap mode so each bit is a pixel.

                for (size_t y = 0; y < glyph->bitmap.rows; ++y) {
                    for (size_t x = 0; x < glyph->bitmap.width; ++x) {
                        int imageIndex = (glyph->bitmap.width * y) + x;

                        if (glyphBit(glyph, x, y)) {
                            bitmap[imageIndex] = 255;
                        } else {
                            bitmap[imageIndex] = 0;
                        }
                    }
                }

                // we can probably do both of this in a single operation but I've had a horrible
                // time doing that. so above, we're wasting a whole byte to represent a bit..

                sf::Image glyph_image;
                glyph_image.create(m_character_width, m_character_height, sf::Color::Transparent);

                for (uint32_t offset_y = 0; offset_y < m_character_height; offset_y++) {
                    for (uint32_t offset_x = 0; offset_x < m_character_width; offset_x++) {
                        uint32_t pixel_offset = 4 * offset_x + offset_y * m_character_width;

                        // if the pixel is set then write the color as white. We will apply
                        // color to the vertex.
                        if (bitmap[offset_x + offset_y * m_character_width] == 255) {
                            glyph_image.setPixel(offset_x, offset_y, sf::Color::White);
                        }
                    }
                }

                // ok so now we have an image with the rendered glyph in white with a transparent bg.
                atlas_offset = setAtlasGlyph(character, glyph_image);
            } else {
                atlas_offset = atlas_offset_it->second;
            }
            setQuadColorForScreenLocation(m_console_bg_vertices, sf::Vector2i(x, y), bg_color);
            setQuadColorForScreenLocation(m_console_fg_vertices, sf::Vector2i(x, y), fg_color);
            setTexCoordsForScreenLocation(m_console_fg_vertices,
                                          sf::Vector2i(x, y),
                                          getAtlasCoordsForOffset(atlas_offset));

            m_console_dirty[x + y * m_width] = false; // it's clean now!
        }
    }

    m_console_bg_vertex_buffer.update(m_console_bg_vertices.data(), m_width * m_height * 4, 0);
    m_console_fg_vertex_buffer.update(m_console_fg_vertices.data(), m_width * m_height * 4, 0);
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
        new_atlas.create(m_atlas_width * m_character_width, m_atlas_width * m_character_height + atlas_size.y);
        new_atlas.update(m_console_atlas, 0, 0); // copy the current atlas over
        new_atlas.setSmooth(false);
        m_console_atlas = new_atlas; // replace with the new atlas.

        SPDLOG_INFO("grew the atlas");
    }

    // get the x,y co-ordinates of the image we are adding to the atlas.
    uint32_t x = (m_glyph_count % m_atlas_width) * m_character_width;
    uint32_t y = (m_glyph_count / m_atlas_width) * m_character_width;

    m_console_atlas.update(image, x, y);
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
    auto font_data = file_cache->Get(font_file);

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

    SPDLOG_INFO("loaded font {}", m_face->family_name);

    return;
}

void ConsoleScreen::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // every draw loop, we are shipping the buffer up; I think we could actually be smarter and only ship a buffer if it
    // changed. but buffers can't easily be partially updated, so its all or nothing.

    target.draw(m_console_bg_vertex_buffer, states);
    states.texture = &m_console_atlas;
    target.draw(m_console_fg_vertex_buffer, states);
}

const void ConsoleScreen::crash()
{
    std::random_device rd;

    for (uint32_t y = 0; y < m_height; y++) {
        for (uint32_t x = 0; x < m_width; x++) {
            uint32_t fg_color = rd() % m_palette_colors.size();
            uint32_t bg_color = rd() % m_palette_colors.size();
            char32_t c        = 33 + rd() % 128;
            poke(sf::Vector2i(x, y), c, fg_color, bg_color);
        }
    }
}

const void ConsoleScreen::loading()
{
    std::random_device rd;

    for (uint32_t y = 0; y < m_height; y++) {
        uint32_t bg_color = rd() % m_palette_colors.size();
        for (uint32_t x = 0; x < m_width; x++) {
            poke(sf::Vector2i(x, y), 32, 0, bg_color);
        }
    }
}

} // namespace Engine
