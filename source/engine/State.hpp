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

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <typeindex>

#include <spdlog/spdlog.h>
#include <SFML/Window.hpp>

namespace ge
{

using event_handler_func = std::function<void(const sf::Event &)>;

class State
{
  public:
    void SetName(const std::string &);

    void ProcessEvent(const sf::Event &);

    template <typename EventType>
    void AddHandler(std::function<void(const EventType &)> func)
    {
        auto key     = std::type_index(typeid(EventType));
        auto wrapper = [f = std::move(func)](const sf::Event &event) {
            if (const auto *e = event.getIf<EventType>())
                f(*e);
        };

        if (event_handlers_.count(key) == 0) {
            auto d = std::make_unique<std::deque<event_handler_func>>();
            d->push_back(wrapper);
            event_handlers_[key] = std::move(d);
            return;
        }

        event_handlers_[key]->push_back(wrapper);
    }

  private:
    std::string name_;
    std::map<std::type_index, std::unique_ptr<std::deque<event_handler_func>>> event_handlers_;
};

} // namespace ge
