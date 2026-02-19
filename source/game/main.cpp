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

#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>

#include "Game.hpp"
#include "GameScreen.hpp"
#include "engine/Engine.hpp"

#ifdef __APPLE__
#include "resourcePath.hpp"
#endif

int main()
{
    SPDLOG_INFO("starting gridengine");

#ifdef __APPLE__
    SPDLOG_INFO("running on mac OS X, resource path: {}", resourcePath());
    std::filesystem::current_path(resourcePath());
#else
    SPDLOG_INFO("current working directory: {}", std::filesystem::current_path().string());
#endif

    auto engine = std::make_unique<gr::Game>();
    auto screen = std::make_unique<gr::GameScreen>();
    engine->create(sf::VideoMode(sf::Vector2u{800, 600}), std::move(screen),
                   "data/unscii-8.pcf", 8, 8, "GridEngine");
    engine->start();

    return EXIT_SUCCESS;
}
