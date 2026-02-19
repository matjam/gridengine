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
#include <mutex>
#include <optional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Event.hpp"

namespace ge
{

class InputQueue
{
  public:
    void install(GLFWwindow *window);
    void push(Event event);
    std::optional<Event> poll();

  private:
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void charCallback(GLFWwindow *window, unsigned int codepoint);
    static void windowCloseCallback(GLFWwindow *window);
    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);

    std::mutex m_mutex;
    std::deque<Event> m_queue;
};

} // namespace ge
