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

#include <string>

namespace Engine::UI
{

struct MenuOption {
    uint32_t id;
    std::string title;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Implements a simple Character based modal menu to be drawn on a ConsoleScreen in the center of the screen.
///
/// Draws a box and rows of options and then allows the user to move up and down the options and select one.
///
/// Also supports a mouse click with hover highlighting.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Menu
{
  public:
    Menu() = default;

    void create(std::string title, std::vector<MenuOption> &options, uint32_t width);
};

} // namespace Engine::UI
