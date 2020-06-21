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
                    const std::string &title)
{
    SPDLOG_INFO("creating engine");

    m_video_mode = video_mode;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    settings.depthBits         = 24;

    m_window =
        std::make_unique<sf::RenderWindow>(m_video_mode, title, sf::Style::Titlebar | sf::Style::Close, settings);
    m_window->setVerticalSyncEnabled(true);

    // set up our ConsoleScreen
    m_screen = std::move(console_screen);

    // all the information related to the screen
    m_screen_width  = m_screen->size().x;
    m_screen_height = m_screen->size().y;
    m_font_width    = m_screen->characterSize().x;
    m_font_height   = m_screen->characterSize().y;
    m_script_engine = std::make_unique<ScriptEngine>();
    m_state_stack   = std::make_unique<StateStack>();
    m_state         = std::make_shared<State>();

    // default state stack
    m_state->SetName("default");
    addDefaultHandlers();
    m_state_stack->Push(m_state);

    // ensure our view is setup correctly
    sf::View view = m_window->getDefaultView();
    view.zoom(1.0 / (m_video_mode.height / (m_font_height * m_screen_height)));
    view.setCenter((m_screen_width * m_font_width) / 2, (m_screen_height * m_font_height) / 2);
    m_window->setView(view);
    m_screen->setPosition(sf::Vector2f(0.0, 0.0));
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
        sf::Event event;
        Stats::begin("process_event");
        while (m_window->pollEvent(event)) {
            m_state_stack->ProcessEvent(event);
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
        m_screen->rectangle(sf::IntRect(1, 39, 22, 5), 32, 1);
        m_screen->write(sf::Vector2i(2, 40), fmt::format("{} fps", 1000000 / Stats::getAverageTime("frame_time")));
        m_screen->write(sf::Vector2i(2, 41),
                        fmt::format("render time {} ms", Stats::getAverageTime("render_time") / 1000));
        m_screen->write(sf::Vector2i(2, 42),
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
    m_state->AddHandler(sf::Event::Closed, [&](const sf::Event &) {
        SPDLOG_INFO("window closed");
        m_window->close();
    });

    m_state->AddHandler(sf::Event::Resized, [&](const sf::Event &event) {
        sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
        m_window->setView(sf::View(visibleArea));
    });

    m_state->AddHandler(sf::Event::KeyPressed, [&](const sf::Event &event) {
        keyEventHandler(event);
    });
}

void Engine::keyEventHandler(const sf::Event &event)
{
    if (event.key.code == sf::Keyboard::PageDown && m_debug_screen == DebugScreen::CHAR_DUMP) {
        m_dump_start += 272;
    }

    if (event.key.code == sf::Keyboard::PageUp && m_debug_screen == DebugScreen::CHAR_DUMP) {
        if (m_dump_start > 304) {
            m_dump_start -= 272;
        } else {
            m_dump_start = 32;
        }

        if (m_dump_start < 32) {
            m_dump_start = 32;
        }
    }

    if (event.key.code == sf::Keyboard::F1) {
        m_fps_overlay = !m_fps_overlay;
        m_screen->clear();
    }

    if (event.key.code == sf::Keyboard::F2) {
        if (m_debug_screen == DebugScreen::CHAR_DUMP) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();
        } else {
            m_debug_screen = DebugScreen::CHAR_DUMP;
            m_screen->clear();
        }
    }

    if (event.key.code == sf::Keyboard::F3) {
        if (m_debug_screen == DebugScreen::CRASH) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();
        } else {
            m_debug_screen = DebugScreen::CRASH;
        }
    }

    if (event.key.code == sf::Keyboard::F4) {
        if (m_debug_screen == DebugScreen::LOADING) {
            m_debug_screen = DebugScreen::NONE;
            m_screen->clear();

        } else {
            m_debug_screen = DebugScreen::LOADING;
        }
    }
}

} // namespace ge
