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

#include "InputQueue.hpp"

namespace ge
{

void InputQueue::install(GLFWwindow *window)
{
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
    glfwSetWindowCloseCallback(window, windowCloseCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
}

void InputQueue::push(Event event)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(std::move(event));
}

std::optional<Event> InputQueue::poll()
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty())
        return std::nullopt;
    Event e = std::move(m_queue.front());
    m_queue.pop_front();
    return e;
}

void InputQueue::keyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int mods)
{
    auto *queue = static_cast<InputQueue *>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        queue->push(event::KeyPressed{
            static_cast<Key>(key),
            (mods & GLFW_MOD_SHIFT) != 0,
            (mods & GLFW_MOD_CONTROL) != 0,
            (mods & GLFW_MOD_ALT) != 0,
            (mods & GLFW_MOD_SUPER) != 0,
        });
    } else if (action == GLFW_RELEASE) {
        queue->push(event::KeyReleased{
            static_cast<Key>(key),
            (mods & GLFW_MOD_SHIFT) != 0,
            (mods & GLFW_MOD_CONTROL) != 0,
            (mods & GLFW_MOD_ALT) != 0,
            (mods & GLFW_MOD_SUPER) != 0,
        });
    }
}

void InputQueue::charCallback(GLFWwindow *window, unsigned int codepoint)
{
    auto *queue = static_cast<InputQueue *>(glfwGetWindowUserPointer(window));
    queue->push(event::TextEntered{codepoint});
}

void InputQueue::windowCloseCallback(GLFWwindow *window)
{
    auto *queue = static_cast<InputQueue *>(glfwGetWindowUserPointer(window));
    queue->push(event::Closed{});
}

void InputQueue::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    auto *queue = static_cast<InputQueue *>(glfwGetWindowUserPointer(window));
    queue->push(event::Resized{static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
}

void InputQueue::mouseButtonCallback(GLFWwindow *window, int button, int action, int /*mods*/)
{
    auto *queue = static_cast<InputQueue *>(glfwGetWindowUserPointer(window));
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (action == GLFW_PRESS) {
        queue->push(event::MouseButtonPressed{button, xpos, ypos});
    } else if (action == GLFW_RELEASE) {
        queue->push(event::MouseButtonReleased{button, xpos, ypos});
    }
}

void InputQueue::cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto *queue = static_cast<InputQueue *>(glfwGetWindowUserPointer(window));
    queue->push(event::MouseMoved{xpos, ypos});
}

} // namespace ge
