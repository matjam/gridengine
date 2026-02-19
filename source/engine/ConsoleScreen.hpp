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
#include <random>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#include "Drawable.hpp"
#include "Types.hpp"

namespace ge
{

struct Vertex {
    Vec2f position;  // 8 bytes
    Color color;     // 4 bytes
    Vec2f texCoords; // 8 bytes
};

struct FontInfo {
    uint32_t width;
    uint32_t height;
    uint32_t glyphs;
};

class ConsoleScreen : public Drawable
{
  public:
    ConsoleScreen();
    virtual ~ConsoleScreen();

    const void create(uint32_t width, uint32_t height, std::string font_file, uint32_t font_width,
                      uint32_t font_height);

    const Vec2i characterSize();
    const Vec2i size();

    const void createPalette(const std::vector<Color> &palette_colors);
    std::vector<Color> palette();

    const void setForeground(uint32_t color);
    const void setBackGround(uint32_t color);

    const void write(Vec2i location, std::string text, uint32_t max_width, uint32_t fg, uint32_t bg);
    const void write(Vec2i location, std::string text, uint32_t max_width, uint32_t fg);
    const void write(Vec2i location, std::string text, uint32_t max_width);
    const void write(Vec2i location, std::string text);
    const void write(uint32_t x, uint32_t y, std::string text);

    const void writeCenter(IntRect, std::string);
    const void writeRectangle(IntRect, std::string text);

    void poke(Vec2i location, char32_t character, uint32_t fg, uint32_t bg);
    void poke(uint32_t, uint32_t, char32_t character, uint32_t fg, uint32_t bg);

    const std::tuple<const char32_t, const uint32_t, const uint32_t> peek(Vec2i location);

    const void rectangle(IntRect bounds, char32_t character, bool filled);

    const void displayCharacterCodes(Vec2i location, char32_t start);

    const void clear();

    const void update();

    void render(const glm::mat4 &projection, const glm::mat4 &model);

    const void crash();
    const void loading();

  private:
    static constexpr int VERTS_PER_CELL = 6;

    void loadFont(std::string font_file, uint32_t pixel_size);
    void initGL();

    void setCellPositions(std::vector<Vertex> &vertices, const Vec2i &location,
                          float left, float top, float width, float height);

    void setCellColor(std::vector<Vertex> &, const Vec2i &, const Color &);

    void setCellTexCoords(std::vector<Vertex> &, const Vec2i &, const Vec2f &);

    uint32_t setAtlasGlyph(char32_t charcode, const std::vector<uint8_t> &rgba_pixels);

    Vec2f getAtlasCoordsForOffset(const uint32_t &);

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_character_height;
    uint32_t m_character_width;
    uint32_t m_current_fg;
    uint32_t m_current_bg;
    uint32_t m_glyph_count = 0;
    uint32_t m_atlas_width = 128; // in characters

    FT_Library m_library;
    FT_Face m_face;

    std::vector<Color> m_palette_colors;

    std::vector<bool> m_console_dirty;
    std::vector<uint8_t> m_console_fg;
    std::vector<uint8_t> m_console_bg;
    std::vector<char32_t> m_console;

    std::vector<Vertex> m_console_bg_vertices;
    std::vector<Vertex> m_console_fg_vertices;

    GLuint m_bg_vao = 0, m_bg_vbo = 0;
    GLuint m_fg_vao = 0, m_fg_vbo = 0;
    GLuint m_atlas_texture = 0;
    uint32_t m_atlas_tex_width = 0;
    uint32_t m_atlas_tex_height = 0;

    std::unordered_map<char32_t, uint32_t> m_console_atlas_offset;
    std::mt19937 m_rng{std::random_device{}()};
};

} // namespace ge
