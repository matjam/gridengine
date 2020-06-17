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

#include "FileCache.hpp"

namespace Engine
{

std::unique_ptr<FileCache> file_cache;

void FileCache::Clear()
{
    file_cache.clear();
}

auto FileCache::Get(const std::filesystem::path &path) -> std::shared_ptr<std::vector<char>>
{
    const std::lock_guard<std::mutex> lock(file_cache_mutex);

    if (file_cache.count(path.string()) != 0) {
        SPDLOG_INFO("Found cached asset {}", path.string());
        return file_cache[path.string()];
    }

    std::ifstream ifs(path, std::ios::binary | std::ios::ate);

    if (!ifs) {
        SPDLOG_ERROR("unable to load asset {}: {}", path.string(), std::strerror(errno));
        return nullptr;
    }

    auto end = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    auto size = std::size_t(end - ifs.tellg());
    if (size == 0) {
        SPDLOG_WARN("file {} is 0 size", path.string());
        return nullptr;
    }
    auto buffer = std::make_shared<std::vector<char>>(size);

    if (!ifs.read(buffer->data(), static_cast<std::streamsize>(buffer->size()))) {
        SPDLOG_ERROR("unable to load asset {}: {}", path.string(), std::strerror(errno));
        return nullptr;
    }

    file_cache[path.string()] = buffer;
    SPDLOG_INFO("Loaded asset {} into cache", path.string());

    return buffer;
}

} // namespace Engine
