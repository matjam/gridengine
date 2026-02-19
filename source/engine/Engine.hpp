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

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>

#include "ConsoleScreen.hpp"
#include "Drawable.hpp"
#include "Event.hpp"
#include "InputQueue.hpp"
#include "Logging.hpp"
#include "ScriptEngine.hpp"
#include "State.hpp"
#include "StateStack.hpp"
#include "Types.hpp"

namespace ge
{

struct EngineConfiguration {
    std::string fontname;
};

class Engine
{
  public:
    enum class DebugScreen { NONE, CHAR_DUMP, LOADING, CRASH };

    Engine();
    virtual ~Engine();

    virtual void create();
    virtual void create(uint32_t window_width, uint32_t window_height,
                        std::unique_ptr<ConsoleScreen> console_screen,
                        const std::string &font_file, uint32_t font_width, uint32_t font_height,
                        const std::string &title);
    virtual void start();

    StateStack &stateStack();
    ConsoleScreen &screen();
    ScriptEngine &scriptEngine();
    State &state();

  protected:
    virtual void render();
    virtual void update();

  private:
    void addDefaultHandlers();
    void renderDebugScreen();
    void keyEventHandler(const event::KeyPressed &);

    char32_t m_dump_start      = 32;
    DebugScreen m_debug_screen = DebugScreen::NONE;
    bool m_fps_overlay         = false;

    uint32_t m_window_width    = 800;
    uint32_t m_window_height   = 600;
    uint32_t m_screen_width    = 80;
    uint32_t m_screen_height   = 45;
    uint32_t m_font_width      = 8;
    uint32_t m_font_height     = 8;
    uint32_t m_frame_count     = 0;

    GLFWwindow *m_window = nullptr;
    InputQueue m_input_queue;
    glm::mat4 m_projection{1.0f};
    glm::mat4 m_model{1.0f};

    std::unique_ptr<ConsoleScreen> m_screen;
    std::unique_ptr<ScriptEngine> m_script_engine;
    std::unique_ptr<StateStack> m_state_stack;
    std::shared_ptr<State> m_state;
};

} // namespace ge
