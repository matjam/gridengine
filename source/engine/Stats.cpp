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

#include "Stats.hpp"

namespace ge
{

uint64_t Stats::m_slice_max;
std::map<std::string, std::deque<uint64_t>> Stats::m_slices;
std::map<std::string, sf::Clock> Stats::m_active_timers;

void Stats::setMaxSlices(uint64_t slice_max)
{
    m_slice_max = slice_max;
}

void Stats::begin(std::string name)
{
    auto timer            = sf::Clock();
    m_active_timers[name] = timer;
}

void Stats::end(std::string name)
{
    auto elapsed_time = m_active_timers[name].getElapsedTime().asMicroseconds();

    m_slices[name].push_back(elapsed_time);

    if (m_slices[name].size() > m_slice_max) {
        m_slices[name].pop_front();
    }
}

uint64_t Stats::getAverageTime(std::string name)
{
    uint64_t total = 0;
    uint64_t count = 0;

    for (auto &value : m_slices[name]) {
        total += value;
        count++;
    }

    if (count == 0)
        return 0;

    return total / count;
}

} // namespace ge
