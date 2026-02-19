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

#include <SFML/Graphics.hpp>

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

    // should probably put this in the Logging class.

    spdlog::set_default_logger(console);

    Stats::setMaxSlices(30);
}

void Engine::create()
{
    // should implement some default behaviour here.
}

void Engine::create(const sf::VideoMode &video_mode, std::unique_ptr<ConsoleScreen> console_screen,
                    const std::string &font_file, uint32_t font_width, uint32_t font_height,
                    const std::string &title)
{
    SPDLOG_INFO("creating engine");

    m_video_mode  = video_mode;
    m_font_width  = font_width;
    m_font_height = font_height;

    sf::ContextSettings settings;
    settings.depthBits = 24;

    m_window =
        std::make_unique<sf::RenderWindow>(m_video_mode, title, sf::Style::Titlebar | sf::Style::Close, sf::State::Windowed, settings);
    m_window->setVerticalSyncEnabled(true);

    // compute grid dimensions from window size — only whole cells
    m_screen_width  = m_video_mode.size.x / m_font_width;
    m_screen_height = m_video_mode.size.y / m_font_height;

    SPDLOG_INFO("grid size: {}x{} cells (font {}x{}, window {}x{})",
                m_screen_width, m_screen_height, m_font_width, m_font_height,
                m_video_mode.size.x, m_video_mode.size.y);

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

    // set up a 1:1 pixel view and center the grid
    uint32_t grid_pixel_w = m_screen_width * m_font_width;
    uint32_t grid_pixel_h = m_screen_height * m_font_height;
    float offset_x = static_cast<float>(m_video_mode.size.x - grid_pixel_w) / 2.0f;
    float offset_y = static_cast<float>(m_video_mode.size.y - grid_pixel_h) / 2.0f;

    sf::View view(sf::FloatRect({0.f, 0.f},
                  {static_cast<float>(m_video_mode.size.x), static_cast<float>(m_video_mode.size.y)}));
    m_window->setView(view);
    m_screen->setPosition(sf::Vector2f{offset_x, offset_y});

    m_screen->setBackGround(0);
    m_screen->setForeground(1);
    m_screen->clear();

    SPDLOG_INFO("engine created");
}

void Engine::start()
{
    SPDLOG_INFO("starting engine loop");

    // Used to calculate FPS.
    Stats::begin("frame_time");

    while (m_window->isOpen()) {
        Stats::begin("process_event");
        while (const auto event = m_window->pollEvent()) {
            m_state_stack->ProcessEvent(*event);
        }
        Stats::end("process_event");

        // execute per frame game logic if any.
        Stats::begin("update_time");
        update();
        Stats::end("update_time");

        // render the things.
        Stats::begin("render_time");
        render();
        Stats::end("render_time");

        m_window->display();
        Stats::end("frame_time");
        Stats::begin("frame_time");

        m_frame_count++;
    }

    SPDLOG_INFO("ending engine loop");

    m_screen.reset();
    m_window.reset();
}

void Engine::renderDebugScreen()
{

    switch (m_debug_screen) {
    case DebugScreen::NONE: break;
    case DebugScreen::CHAR_DUMP:
        m_screen->clear();
        m_screen->displayCharacterCodes(sf::Vector2i(4, 4), m_dump_start);
        break;
    case DebugScreen::CRASH: m_screen->crash(); break;
    case DebugScreen::LOADING: m_screen->loading(); break;
    }

    if (m_fps_overlay) {
        int oy = static_cast<int>(m_screen_height) - 6;
        m_screen->rectangle(sf::IntRect(sf::Vector2i{1, oy}, sf::Vector2i{22, 5}), 32, 1);
        m_screen->write(sf::Vector2i(2, oy + 1), fmt::format("{} fps", 1000000 / Stats::getAverageTime("frame_time")));
        m_screen->write(sf::Vector2i(2, oy + 2),
                        fmt::format("render time {} ms", Stats::getAverageTime("render_time") / 1000));
        m_screen->write(sf::Vector2i(2, oy + 3),
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
    m_window->draw(*m_screen);
}

void Engine::update()
{
}

void Engine::addDefaultHandlers()
{
    m_state->AddHandler<sf::Event::Closed>([&](const sf::Event::Closed &) {
        SPDLOG_INFO("window closed");
        m_window->close();
    });

    m_state->AddHandler<sf::Event::Resized>([&](const sf::Event::Resized &resized) {
        sf::FloatRect visibleArea(sf::Vector2f{0.f, 0.f},
                                  sf::Vector2f{static_cast<float>(resized.size.x), static_cast<float>(resized.size.y)});
        m_window->setView(sf::View(visibleArea));
    });

    m_state->AddHandler<sf::Event::KeyPressed>([&](const sf::Event::KeyPressed &keyPress) {
        keyEventHandler(keyPress);
    });
}

void Engine::keyEventHandler(const sf::Event::KeyPressed &keyPress)
{
    if (keyPress.code == sf::Keyboard::Key::PageDown && m_debug_screen == DebugScreen::CHAR_DUMP) {
        m_dump_start += 272;
    }

    if (keyPress.code == sf::Keyboard::Key::PageUp && m_debug_screen == DebugScreen::CHAR_DUMP) {
        if (m_dump_start > 304) {
            m_dump_start -= 272;
        } else {
            m_dump_start = 32;
        }

        if (m_dump_start < 32) {
            m_dump_start = 32;
        }
    }

    if (keyPress.code == sf::Keyboard::Key::F1) {
        m_fps_overlay = !m_fps_overlay;
        m_screen->clear();
    }

    if (keyPress.code == sf::Keyboard::Key::F2) {
        if (m_debug_screen == DebugScreen::CHAR_DUMP) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();
        } else {
            m_debug_screen = DebugScreen::CHAR_DUMP;
            m_screen->clear();
        }
    }

    if (keyPress.code == sf::Keyboard::Key::F3) {
        if (m_debug_screen == DebugScreen::CRASH) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();
        } else {
            m_debug_screen = DebugScreen::CRASH;
        }
    }

    if (keyPress.code == sf::Keyboard::Key::F4) {
        if (m_debug_screen == DebugScreen::LOADING) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();

        } else {
            m_debug_screen = DebugScreen::LOADING;
        }
    }
}

} // namespace ge
