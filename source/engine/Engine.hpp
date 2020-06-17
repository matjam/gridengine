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

#include <memory>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include "ConsoleScreen.hpp"
#include "Drawable.hpp"
#include "Logging.hpp"
#include "ScriptEngine.hpp"
#include "State.hpp"
#include "StateStack.hpp"

namespace ge
{

class Engine
{
  public:
    enum class DebugScreen { NONE, CHAR_DUMP, LOADING, CRASH };

    Engine();
    virtual ~Engine() = default;

    void create(const sf::VideoMode &, std::unique_ptr<ConsoleScreen>, const std::string &);
    void start();

    StateStack &stateStack();
    ConsoleScreen &screen();
    ScriptEngine &scriptEngine();

  protected:
    virtual void render(); // implemented by derived classes for custom rendering.
    virtual void update(); // implemented by derived classes to implement any logic.

  private:
    void addDefaultHandlers();
    void renderDebugScreen();
    void keyEventHandler(const sf::Event &);

    char32_t m_dump_start      = 32;
    DebugScreen m_debug_screen = DebugScreen::NONE;
    bool m_fps_overlay         = false;

    sf::VideoMode m_video_mode = {1920, 1080};
    uint32_t m_screen_width    = 80;
    uint32_t m_screen_height   = 45;
    uint32_t m_font_width      = 8;
    uint32_t m_font_height     = 8;
    uint32_t m_frame_count     = 0;

    std::unique_ptr<sf::RenderWindow> m_window;
    std::unique_ptr<ConsoleScreen> m_screen;
    std::unique_ptr<ScriptEngine> m_script_engine;
    std::unique_ptr<StateStack> m_state_stack;
    std::shared_ptr<State> m_state;
};

} // namespace ge
