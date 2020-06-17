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

#include "State.hpp"

namespace ge
{

void State::SetName(const std::string &name)
{
    name_ = name;
    SPDLOG_INFO("State [{}] named", name);
}

// runs each handler in the order in which it was added.
void State::ProcessEvent(sf::Event &event)
{
    auto type = static_cast<int>(event.type);
    if (event_handlers_.count(type) == 0)
        return; // no registered handler for that event.

    for (auto &handler : *event_handlers_[type]) {
        handler(event);
    }
}

// add an event handler like so:
//
// add_event_handler(sf::Event::MouseButtonPressed, [](const sf::Event &event) {
//     spdlog::info("mouse button pressed!");
// });

// add a given event handler to the event handler map.
void State::AddHandler(sf::Event::EventType event_type, const event_handler_func &func)
{
    auto type = static_cast<int>(event_type);
    if (event_handlers_.count(type) == 0) {
        auto d = std::make_unique<std::deque<event_handler_func>>();
        d->push_back(func);
        event_handlers_[type] = move(d);
        return;
    }

    event_handlers_[type]->push_back(func);
}

} // namespace ge
