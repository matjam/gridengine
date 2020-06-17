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

#include <string>
#include <memory>

#include "ScriptEngine.hpp"

#include "Logging.hpp"
#include <spdlog/spdlog.h>

namespace Engine
{
ScriptEngine::ScriptEngine()
{
    lua = std::make_unique<sol::state>();

    // open some common libraries
    lua->open_libraries(sol::lib::base, sol::lib::package);

    lua->set_function("logInfo", &ScriptEngine::luaLogInfo, this);

    lua->script_file("data/scripts/engine.lua");
}

void ScriptEngine::luaLogInfo(std::string message)
{
    auto lua_state = lua->lua_state();
    lua_Debug ar;
    lua_getstack(lua_state, 0, &ar);
    lua_getinfo(lua_state, "nSl", &ar);
    console->log(spdlog::source_loc(ar.name, ar.currentline, ar.name), spdlog::level::info, message);
}

void ScriptEngine::getEngineConfiguration()
{
}
} // namespace Engine
