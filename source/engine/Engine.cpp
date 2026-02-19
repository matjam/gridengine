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

#include "Engine.hpp"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>
#include <random>

#include <math.h> /* ceil */

#include <glm/gtc/matrix_transform.hpp>

#include "FileCache.hpp"
#include "State.hpp"
#include "Stats.hpp"

namespace ge
{

Engine::Engine()
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("gridrunner.log", 1000000, 5, true));
    console = std::make_shared<spdlog::logger>("console", begin(sinks), end(sinks));

    spdlog::set_default_logger(console);

    Stats::setMaxSlices(30);
}

Engine::~Engine()
{
    m_screen.reset();
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Engine::create()
{
    // should implement some default behaviour here.
}

void Engine::create(uint32_t window_width, uint32_t window_height,
                    std::unique_ptr<ConsoleScreen> console_screen,
                    const std::string &font_file, uint32_t font_width, uint32_t font_height,
                    const std::string &title)
{
    SPDLOG_INFO("creating engine");

    m_window_width  = window_width;
    m_window_height = window_height;
    m_font_width    = font_width;
    m_font_height   = font_height;

    // Initialize GLFW
    if (!glfwInit()) {
        SPDLOG_ERROR("failed to initialize GLFW");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(m_window_width, m_window_height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        SPDLOG_ERROR("failed to create GLFW window");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);

    // Initialize GLAD — must be after glfwMakeContextCurrent
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        SPDLOG_ERROR("failed to initialize GLAD");
        glfwDestroyWindow(m_window);
        glfwTerminate();
        m_window = nullptr;
        return;
    }

    glfwSwapInterval(1); // vsync

    // Install input callbacks
    m_input_queue.install(m_window);

    // compute grid dimensions from window size — only whole cells
    m_screen_width  = m_window_width / m_font_width;
    m_screen_height = m_window_height / m_font_height;

    SPDLOG_INFO("grid size: {}x{} cells (font {}x{}, window {}x{})",
                m_screen_width, m_screen_height, m_font_width, m_font_height,
                m_window_width, m_window_height);

    // create the console screen with the computed dimensions
    m_screen = std::move(console_screen);
    m_screen->create(m_screen_width, m_screen_height, font_file, font_width, font_height);

    m_script_engine = std::make_unique<ScriptEngine>();
    m_state_stack   = std::make_unique<StateStack>();
    m_state         = std::make_shared<State>();

    // default state stack
    m_state->SetName("default");
    addDefaultHandlers();
    m_state_stack->Push(m_state);

    // set up orthographic projection: top-left origin, Y-down (matches SFML's coordinate system)
    m_projection = glm::ortho(0.0f, static_cast<float>(m_window_width),
                              static_cast<float>(m_window_height), 0.0f,
                              -1.0f, 1.0f);

    // center the grid in the window
    uint32_t grid_pixel_w = m_screen_width * m_font_width;
    uint32_t grid_pixel_h = m_screen_height * m_font_height;
    float offset_x = static_cast<float>(m_window_width - grid_pixel_w) / 2.0f;
    float offset_y = static_cast<float>(m_window_height - grid_pixel_h) / 2.0f;

    m_screen->setPosition(Vec2f{offset_x, offset_y});
    m_model = m_screen->getTransform();

    m_screen->setBackGround(0);
    m_screen->setForeground(1);
    m_screen->clear();

    // Set viewport to framebuffer size (may differ from window size on HiDPI/Wayland)
    int fb_width, fb_height;
    glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    SPDLOG_INFO("framebuffer size: {}x{} (window: {}x{})", fb_width, fb_height, m_window_width, m_window_height);

    // Enable blending for glyph transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SPDLOG_INFO("engine created");
}

void Engine::start()
{
    SPDLOG_INFO("starting engine loop");

    Stats::begin("frame_time");

    while (!glfwWindowShouldClose(m_window)) {
        Stats::begin("process_event");
        glfwPollEvents();
        while (auto event = m_input_queue.poll()) {
            m_state_stack->ProcessEvent(*event);
        }
        Stats::end("process_event");

        // execute per frame game logic if any.
        Stats::begin("update_time");
        update();
        Stats::end("update_time");

        // render the things.
        Stats::begin("render_time");
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        render();
        Stats::end("render_time");

        glfwSwapBuffers(m_window);
        Stats::end("frame_time");
        Stats::begin("frame_time");

        m_frame_count++;
    }

    SPDLOG_INFO("ending engine loop");

    m_screen.reset();
}

void Engine::renderDebugScreen()
{

    switch (m_debug_screen) {
    case DebugScreen::NONE: break;
    case DebugScreen::CHAR_DUMP:
        m_screen->clear();
        m_screen->displayCharacterCodes(Vec2i(4, 4), m_dump_start);
        break;
    case DebugScreen::CRASH: m_screen->crash(); break;
    case DebugScreen::LOADING: m_screen->loading(); break;
    }

    if (m_fps_overlay) {
        int oy = static_cast<int>(m_screen_height) - 6;
        m_screen->rectangle(IntRect(Vec2i{1, oy}, Vec2i{22, 5}), 32, 1);
        m_screen->write(Vec2i(2, oy + 1), fmt::format("{} fps", 1000000 / Stats::getAverageTime("frame_time")));
        m_screen->write(Vec2i(2, oy + 2),
                        fmt::format("render time {} ms", Stats::getAverageTime("render_time") / 1000));
        m_screen->write(Vec2i(2, oy + 3),
                        fmt::format("update time {} ms", Stats::getAverageTime("update_time") / 1000));
    }
}

StateStack &Engine::stateStack()
{
    return *m_state_stack;
}

ConsoleScreen &Engine::screen()
{
    return *m_screen;
}

ScriptEngine &Engine::scriptEngine()
{
    return *m_script_engine;
}

State &Engine::state()
{
    return *m_state;
}

void Engine::render()
{
    renderDebugScreen();
    m_screen->update();
    m_screen->render(m_projection, m_model);
}

void Engine::update()
{
}

void Engine::addDefaultHandlers()
{
    m_state->AddHandler<event::Closed>([&](const event::Closed &) {
        SPDLOG_INFO("window closed");
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    });

    m_state->AddHandler<event::Resized>([&](const event::Resized &resized) {
        glViewport(0, 0, resized.width, resized.height);
    });

    m_state->AddHandler<event::KeyPressed>([&](const event::KeyPressed &keyPress) {
        keyEventHandler(keyPress);
    });
}

void Engine::keyEventHandler(const event::KeyPressed &keyPress)
{
    if (keyPress.code == Key::PageDown && m_debug_screen == DebugScreen::CHAR_DUMP) {
        m_dump_start += 272;
    }

    if (keyPress.code == Key::PageUp && m_debug_screen == DebugScreen::CHAR_DUMP) {
        if (m_dump_start > 304) {
            m_dump_start -= 272;
        } else {
            m_dump_start = 32;
        }

        if (m_dump_start < 32) {
            m_dump_start = 32;
        }
    }

    if (keyPress.code == Key::F1) {
        m_fps_overlay = !m_fps_overlay;
        m_screen->clear();
    }

    if (keyPress.code == Key::F2) {
        if (m_debug_screen == DebugScreen::CHAR_DUMP) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();
        } else {
            m_debug_screen = DebugScreen::CHAR_DUMP;
            m_screen->clear();
        }
    }

    if (keyPress.code == Key::F3) {
        if (m_debug_screen == DebugScreen::CRASH) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();
        } else {
            m_debug_screen = DebugScreen::CRASH;
        }
    }

    if (keyPress.code == Key::F4) {
        if (m_debug_screen == DebugScreen::LOADING) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();

        } else {
            m_debug_screen = DebugScreen::LOADING;
        }
    }
}

} // namespace ge
