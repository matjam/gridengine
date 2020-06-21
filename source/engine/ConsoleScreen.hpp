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

#include <bitset>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <SFML/Graphics.hpp>
#include <boost/compute/detail/lru_cache.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#include "Drawable.hpp"

namespace ge
{

struct FontInfo {
    uint32_t width;
    uint32_t height;
    uint32_t glyphs;
};

// Class that renders characters into a console style grid with a FG and BG color.
//
// When update() is called, we scan through each character and look in the m_glyph_images map for
// the image that holds the glyph. If we don't find one, we construct it from the sf::Font that we
// loaded on start. This is slow for the first time we render that character, but is cached after
// that so should be fast.
//
// These images are then rendered directly to the texture backing the m_console_sprite.
//
// Obviously, only for fixed width fonts.

class ConsoleScreen : public Drawable
{
  public:
    ConsoleScreen();

    const void create(uint32_t width, uint32_t height, std::string font_file, uint32_t font_width,
                      uint32_t font_height);

    const sf::Vector2i characterSize();

    const sf::Vector2i size();

    // crete a palette from a vector of sf::Colors
    const void createPalette(const std::vector<sf::Color> &palette_colors);

    // gets the current palette
    std::vector<sf::Color> palette();

    // sets the colors for write/draw operations that don't specify a color.
    const void setForeground(uint32_t color);
    const void setBackGround(uint32_t color);

    // writes the given string to a location
    const void write(sf::Vector2i location, std::string text, uint32_t max_width, uint32_t fg, uint32_t bg);
    const void write(sf::Vector2i location, std::string text, uint32_t max_width, uint32_t fg);
    const void write(sf::Vector2i location, std::string text, uint32_t max_width);
    const void write(sf::Vector2i location, std::string text);
    const void write(uint32_t x, uint32_t y, std::string text);

    const void writeCenter(sf::IntRect, std::string);

    const void writeRectangle(sf::IntRect, std::string text);

    // single character access at a location
    void poke(sf::Vector2i location, char32_t character, uint32_t fg, uint32_t bg);
    void poke(uint32_t, uint32_t, char32_t character, uint32_t fg, uint32_t bg);

    const std::tuple<const char32_t, const uint32_t, const uint32_t> peek(sf::Vector2i location);

    // will draw a box, filled or not, or a line of the given unicode character.
    const void rectangle(sf::IntRect bounds, char32_t character, bool filled);

    const void displayCharacterCodes(sf::Vector2i location, char32_t start);

    const void clear();

    // render the screen to the texture.
    const void update();

    const void crash();

    const void loading();

  private:
    struct Quad {
        sf::Vertex &a;
        sf::Vertex &b;
        sf::Vertex &c;
        sf::Vertex &d;
    };

    void loadFont(std::string font_file, uint32_t pixel_size);

    // implements sf::Drawable::draw()
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    struct Quad getQuadForScreenLocation(std::vector<sf::Vertex> &, const sf::Vector2i &);

    void setQuadColorForScreenLocation(std::vector<sf::Vertex> &, const sf::Vector2i &, const sf::Color &);

    void setTexCoordsForScreenLocation(std::vector<sf::Vertex> &, const sf::Vector2i &, const sf::Vector2f &);

    // sets a glyph in the texture atlas to a given sf:image; growing it if required.
    // returns the glyph index in the texture
    uint32_t setAtlasGlyph(char32_t charcode, const sf::Image &image);

    sf::Vector2f getAtlasCoordsForOffset(const uint32_t &);

    uint32_t m_width;             // width in characters of the console
    uint32_t m_height;            // height in characters of the console
    uint32_t m_character_height;  // pixel height of font
    uint32_t m_character_width;   // pixel width of font
    uint32_t m_current_fg;        // current fg color
    uint32_t m_current_bg;        // current bg color
    uint32_t m_glyph_count = 0;   // the number of glphys in the atlas
    uint32_t m_atlas_width = 128; // in characters

    FT_Library m_library; // FreeType font library
    FT_Face m_face;       // FreeType font face

    std::vector<sf::Color> m_palette_colors; // a vector of colors for the palette

    std::vector<bool> m_console_dirty; // how bad is this, really?
    std::vector<uint8_t> m_console_fg; // offset to a palette color
    std::vector<uint8_t> m_console_bg; // offset to a palette color
    std::vector<char32_t> m_console;   // all the actual characters as a unicode code point.

    std::vector<sf::Vertex> m_console_bg_vertices;       // all the vertexes for the background colors
    std::vector<sf::Vertex> m_console_fg_vertices;       // foreground color of the rendered glyphs
    sf::VertexBuffer m_console_bg_vertex_buffer;         // a vertex buffer for the background shapes
    sf::VertexBuffer m_console_fg_vertex_buffer;         // a vertex buffer to render the glyphs with
    sf::Texture m_console_atlas;                         // all of the glyphs we use in an atlas
    std::map<char32_t, uint32_t> m_console_atlas_offset; // a map of the charcode to a given atlas offset.
};

} // namespace ge
