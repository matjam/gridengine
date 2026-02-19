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
#include "Shader.hpp"

#include "FileCache.hpp"

namespace ge
{

ConsoleScreen::ConsoleScreen()
{
}

ConsoleScreen::~ConsoleScreen()
{
    if (m_bg_vao) glDeleteVertexArrays(1, &m_bg_vao);
    if (m_bg_vbo) glDeleteBuffers(1, &m_bg_vbo);
    if (m_fg_vao) glDeleteVertexArrays(1, &m_fg_vao);
    if (m_fg_vbo) glDeleteBuffers(1, &m_fg_vbo);
    if (m_atlas_texture) glDeleteTextures(1, &m_atlas_texture);
}

const void ConsoleScreen::create(uint32_t width, uint32_t height, std::string font_file, uint32_t font_width,
                                 uint32_t font_height)
{
    m_width            = width;
    m_height           = height;
    m_character_height = font_height;
    m_character_width  = font_width;

    loadFont(font_file, m_character_height);

    m_console_dirty.resize(m_width * m_height, true);
    m_console_bg.resize(m_width * m_height, 0);
    m_console_fg.resize(m_width * m_height, 2);
    m_console.resize(m_width * m_height, 32);

    m_console_bg_vertices.resize(m_width * m_height * VERTS_PER_CELL);
    m_console_fg_vertices.resize(m_width * m_height * VERTS_PER_CELL);

    // some default C64 palette colors.
    m_palette_colors = {Color(0, 0, 0),
                        Color(255, 255, 255),
                        Color(136, 0, 0),
                        Color(170, 255, 238),
                        Color(204, 68, 204),
                        Color(0, 204, 85),
                        Color(0, 0, 170),
                        Color(238, 238, 119),
                        Color(221, 136, 85),
                        Color(102, 68, 0),
                        Color(255, 119, 119),
                        Color(51, 51, 51),
                        Color(119, 119, 119),
                        Color(170, 255, 102),
                        Color(0, 136, 255),
                        Color(187, 187, 187)};

    // set up the vertex buffer locations - we only do this once.
    for (uint32_t y = 0; y < m_height; y++) {
        for (uint32_t x = 0; x < m_width; x++) {
            float v_left   = static_cast<float>(x * m_character_width);
            float v_top    = static_cast<float>(y * m_character_height);
            float v_width  = static_cast<float>(m_character_width);
            float v_height = static_cast<float>(m_character_height);

            setCellPositions(m_console_bg_vertices, Vec2i(x, y), v_left, v_top, v_width, v_height);
            setCellPositions(m_console_fg_vertices, Vec2i(x, y), v_left, v_top, v_width, v_height);
        }
    }

    initGL();
}

void ConsoleScreen::initGL()
{
    const uint32_t total = m_width * m_height;
    const size_t buf_size = total * VERTS_PER_CELL * sizeof(Vertex);

    // Background VAO/VBO
    glGenVertexArrays(1, &m_bg_vao);
    glGenBuffers(1, &m_bg_vbo);
    glBindVertexArray(m_bg_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_bg_vbo);
    glBufferData(GL_ARRAY_BUFFER, buf_size, m_console_bg_vertices.data(), GL_STREAM_DRAW);

    // position: location 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // color: location 1 (normalized uint8)
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    // texcoords: location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);

    // Foreground VAO/VBO
    glGenVertexArrays(1, &m_fg_vao);
    glGenBuffers(1, &m_fg_vbo);
    glBindVertexArray(m_fg_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_fg_vbo);
    glBufferData(GL_ARRAY_BUFFER, buf_size, m_console_fg_vertices.data(), GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Create initial atlas texture (small, will grow)
    m_atlas_tex_width  = m_atlas_width * m_character_width;
    m_atlas_tex_height = m_atlas_width * m_character_height;

    glGenTextures(1, &m_atlas_texture);
    glBindTexture(GL_TEXTURE_2D, m_atlas_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_atlas_tex_width, m_atlas_tex_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

const Vec2i ConsoleScreen::characterSize()
{
    return Vec2i(m_character_width, m_character_height);
}

const Vec2i ConsoleScreen::size()
{
    return Vec2i(m_width, m_height);
}

inline void ConsoleScreen::setCellPositions(std::vector<Vertex> &vertices, const Vec2i &location,
                                            float left, float top, float width, float height)
{
    auto offset = (location.x + location.y * m_width) * VERTS_PER_CELL;

    Vec2f tl(left, top);
    Vec2f tr(left + width, top);
    Vec2f br(left + width, top + height);
    Vec2f bl(left, top + height);

    // Triangle 1: top-left, top-right, bottom-right
    vertices[offset + 0].position = tl;
    vertices[offset + 1].position = tr;
    vertices[offset + 2].position = br;
    // Triangle 2: top-left, bottom-right, bottom-left
    vertices[offset + 3].position = tl;
    vertices[offset + 4].position = br;
    vertices[offset + 5].position = bl;
}

inline void ConsoleScreen::setCellColor(std::vector<Vertex> &vertices,
                                        const Vec2i &location, const Color &color)
{
    auto offset = (location.x + location.y * m_width) * VERTS_PER_CELL;

    for (int i = 0; i < VERTS_PER_CELL; i++) {
        vertices[offset + i].color = color;
    }
}

inline void ConsoleScreen::setCellTexCoords(std::vector<Vertex> &vertices,
                                            const Vec2i &location, const Vec2f &texCoords)
{
    auto offset = (location.x + location.y * m_width) * VERTS_PER_CELL;

    // Normalized tex coords (0..1)
    float u0 = texCoords.x;
    float v0 = texCoords.y;
    float u1 = texCoords.x + static_cast<float>(m_character_width) / m_atlas_tex_width;
    float v1 = texCoords.y + static_cast<float>(m_character_height) / m_atlas_tex_height;

    // Triangle 1
    vertices[offset + 0].texCoords = {u0, v0};
    vertices[offset + 1].texCoords = {u1, v0};
    vertices[offset + 2].texCoords = {u1, v1};
    // Triangle 2
    vertices[offset + 3].texCoords = {u0, v0};
    vertices[offset + 4].texCoords = {u1, v1};
    vertices[offset + 5].texCoords = {u0, v1};
}

const void ConsoleScreen::createPalette(const std::vector<Color> &palette_colors)
{
    m_palette_colors = palette_colors;
}

const void ConsoleScreen::setForeground(const uint32_t color)
{
    m_current_fg = color;
}

const void ConsoleScreen::setBackGround(const uint32_t color)
{
    m_current_bg = color;
}

const void ConsoleScreen::write(const Vec2i location, const std::string text, const uint32_t max_width,
                                const uint32_t fg, const uint32_t bg)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    auto str = cv.from_bytes(text);

    uint32_t offset = 0;
    for (auto &c : str) {
        if (offset == max_width)
            break;

        if (location.x + static_cast<int>(offset) >= static_cast<int>(m_width))
            break;

        poke(Vec2i(location.x + offset, location.y), c, fg, bg);
        offset++;
    }
}

const void ConsoleScreen::clear()
{
    rectangle(IntRect(Vec2i{0, 0}, Vec2i{static_cast<int>(m_width), static_cast<int>(m_height)}), 32, 1);
}

const void ConsoleScreen::write(const Vec2i location, const std::string text, const uint32_t max_width,
                                const uint32_t fg)
{
    write(location, text, max_width, fg, m_current_bg);
}

const void ConsoleScreen::write(const Vec2i location, const std::string text, const uint32_t max_width)
{
    write(location, text, max_width, m_current_fg, m_current_bg);
}

const void ConsoleScreen::write(const Vec2i location, const std::string text)
{
    uint32_t max_width = m_width - location.x;
    write(location, text, max_width, m_current_fg, m_current_bg);
}

const void ConsoleScreen::write(const uint32_t x, uint32_t y, const std::string text)
{
    write(Vec2i(x, y), text);
}

const void ConsoleScreen::writeCenter(const IntRect bounds, const std::string text)
{
    auto offset = (bounds.size.x - static_cast<int>(text.length())) / 2;
    write(Vec2i(bounds.position.x + offset, bounds.position.y), text, static_cast<uint32_t>(bounds.size.x));
}

inline void ConsoleScreen::poke(const Vec2i location, const char32_t character, const uint32_t fg,
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
    poke(Vec2i(x, y), character, fg, bg);
}

inline const std::tuple<const char32_t, const uint32_t, const uint32_t> ConsoleScreen::peek(const Vec2i location)
{
    return std::make_tuple(m_console[location.x + location.y * m_width],
                           m_console_fg[location.x + location.y * m_width],
                           m_console_bg[location.x + location.y * m_width]);
}

const void ConsoleScreen::rectangle(const IntRect bounds, const char32_t character, const bool filled)
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
                    poke(Vec2i(x, y), character, m_current_fg, m_current_bg);
                }
            }
        }
    } else {
        for (int x = bounds.position.x; x < bounds.position.x + bounds.size.x; x++) {
            poke(Vec2i(x, bounds.position.y), character, m_current_fg, m_current_bg);
            poke(Vec2i(x, bounds.position.y + bounds.size.y - 1), character, m_current_fg, m_current_bg);
        }
        for (int y = bounds.position.y + 1; y < bounds.position.y + bounds.size.y - 1; y++) {
            poke(Vec2i(bounds.position.x, y), character, m_current_fg, m_current_bg);
            poke(Vec2i(bounds.position.x + bounds.size.x - 1, y), character, m_current_fg, m_current_bg);
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

const void ConsoleScreen::writeRectangle(const IntRect bounds, const std::string text)
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

        poke(Vec2i(x, y), *it, m_current_fg, m_current_bg);
        x++;
        it++;
    }
}

const void ConsoleScreen::displayCharacterCodes(const Vec2i location, const char32_t start)
{
    uint32_t x = location.x;
    uint32_t y = location.y;

    for (char32_t c = start; x < m_width - 10; c++) {
        poke(Vec2i(x, y), c, 1, 11);
        write(Vec2i(x + 2, y), fmt::format("= {0:x}", static_cast<uint32_t>(c)));
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

const void ConsoleScreen::update()
{
    std::vector<uint8_t> bitmap;
    bool any_dirty = false;

    const uint32_t total = m_width * m_height;

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

            // Build RGBA pixel buffer for this glyph
            std::vector<uint8_t> rgba_pixels(m_character_width * m_character_height * 4, 0);

            for (uint32_t offset_y = 0; offset_y < m_character_height; offset_y++) {
                for (uint32_t offset_x = 0; offset_x < m_character_width; offset_x++) {
                    if (bitmap[offset_x + offset_y * m_character_width] == 255) {
                        size_t px = (offset_x + offset_y * m_character_width) * 4;
                        rgba_pixels[px + 0] = 255;
                        rgba_pixels[px + 1] = 255;
                        rgba_pixels[px + 2] = 255;
                        rgba_pixels[px + 3] = 255;
                    }
                }
            }

            atlas_offset = setAtlasGlyph(character, rgba_pixels);
        } else {
            atlas_offset = atlas_offset_it->second;
        }

        const int base = cell * VERTS_PER_CELL;

        // Set background colors
        for (int i = 0; i < VERTS_PER_CELL; i++)
            m_console_bg_vertices[base + i].color = bg_color;

        // Set foreground colors
        for (int i = 0; i < VERTS_PER_CELL; i++)
            m_console_fg_vertices[base + i].color = fg_color;

        // Set foreground tex coords — normalized to 0..1
        const float tx = static_cast<float>((atlas_offset % m_atlas_width) * m_character_width) / m_atlas_tex_width;
        const float ty = static_cast<float>((atlas_offset / m_atlas_width) * m_character_height) / m_atlas_tex_height;
        const float tw = static_cast<float>(m_character_width) / m_atlas_tex_width;
        const float th = static_cast<float>(m_character_height) / m_atlas_tex_height;

        // Triangle 1: TL, TR, BR
        m_console_fg_vertices[base + 0].texCoords = {tx, ty};
        m_console_fg_vertices[base + 1].texCoords = {tx + tw, ty};
        m_console_fg_vertices[base + 2].texCoords = {tx + tw, ty + th};
        // Triangle 2: TL, BR, BL
        m_console_fg_vertices[base + 3].texCoords = {tx, ty};
        m_console_fg_vertices[base + 4].texCoords = {tx + tw, ty + th};
        m_console_fg_vertices[base + 5].texCoords = {tx, ty + th};

        m_console_dirty[cell] = false;
    }

    if (any_dirty) {
        const size_t buf_size = total * VERTS_PER_CELL * sizeof(Vertex);

        glBindBuffer(GL_ARRAY_BUFFER, m_bg_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, buf_size, m_console_bg_vertices.data());

        glBindBuffer(GL_ARRAY_BUFFER, m_fg_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, buf_size, m_console_fg_vertices.data());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

uint32_t ConsoleScreen::setAtlasGlyph(const char32_t charcode, const std::vector<uint8_t> &rgba_pixels)
{
    uint32_t atlas_capacity = (m_atlas_tex_width / m_character_width) * (m_atlas_tex_height / m_character_height);

    if (m_glyph_count == atlas_capacity) {
        // Read back old atlas pixels
        uint32_t old_height = m_atlas_tex_height;
        std::vector<uint8_t> old_pixels(m_atlas_tex_width * old_height * 4);
        glBindTexture(GL_TEXTURE_2D, m_atlas_texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, old_pixels.data());

        // Grow by 128 characters high
        uint32_t new_height = old_height + m_atlas_width * m_character_height;

        GLuint new_tex;
        glGenTextures(1, &new_tex);
        glBindTexture(GL_TEXTURE_2D, new_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_atlas_tex_width, new_height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Copy old data back
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_atlas_tex_width, old_height,
                        GL_RGBA, GL_UNSIGNED_BYTE, old_pixels.data());

        glDeleteTextures(1, &m_atlas_texture);
        m_atlas_texture = new_tex;
        m_atlas_tex_height = new_height;

        SPDLOG_INFO("grew the atlas to {}x{}", m_atlas_tex_width, m_atlas_tex_height);
    }

    uint32_t x = (m_glyph_count % m_atlas_width) * m_character_width;
    uint32_t y = (m_glyph_count / m_atlas_width) * m_character_height;

    glBindTexture(GL_TEXTURE_2D, m_atlas_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, m_character_width, m_character_height,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgba_pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    m_console_atlas_offset[charcode] = m_glyph_count;
    auto rv = m_glyph_count;
    m_glyph_count++;
    return rv;
}

inline Vec2f ConsoleScreen::getAtlasCoordsForOffset(const uint32_t &offset)
{
    float x = static_cast<float>((offset % m_atlas_width) * m_character_width) / m_atlas_tex_width;
    float y = static_cast<float>((offset / m_atlas_width) * m_character_height) / m_atlas_tex_height;

    return Vec2f(x, y);
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

void ConsoleScreen::render(const glm::mat4 &projection, const glm::mat4 &model)
{
    static ShaderProgram shader;
    static bool shader_compiled = false;

    if (!shader_compiled) {
        shader.compile(shaders::kVertexSource, shaders::kFragmentSource);
        shader_compiled = true;
    }

    shader.use();
    shader.setMat4("uProjection", projection);
    shader.setMat4("uModel", model);

    const uint32_t vertex_count = m_width * m_height * VERTS_PER_CELL;

    // Pass 1: background (no texture)
    shader.setBool("uUseTexture", false);
    glBindVertexArray(m_bg_vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    // Pass 2: foreground (with atlas texture)
    shader.setBool("uUseTexture", true);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_atlas_texture);
    shader.setInt("uAtlas", 0);
    glBindVertexArray(m_fg_vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
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

std::vector<Color> ConsoleScreen::palette()
{
    return m_palette_colors;
}

} // namespace ge
