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
#include <exception>
#include <vector>

namespace ge
{

std::mutex FileCache::m_file_cache_mutex;
std::map<std::string, std::shared_ptr<std::vector<char>>> FileCache::m_file_cache;

void FileCache::Clear()
{
    FileCache::m_file_cache.clear();
}

const std::shared_ptr<std::vector<char>> FileCache::Get(const std::filesystem::path &path)
{
    const std::lock_guard<std::mutex> lock(FileCache::m_file_cache_mutex);

    if (FileCache::m_file_cache.count(path.string()) != 0) {
        SPDLOG_INFO("Found cached asset {}", path.string());
        return FileCache::m_file_cache[path.string()];
    }

    try {
        std::ifstream ifs(path, std::ios::binary | std::ios::ate);
        ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        auto end = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        auto size = std::size_t(end - ifs.tellg());

        if (size == 0) {
            SPDLOG_WARN("file {} is 0 size", path.string());
            return nullptr;
        }

        auto buffer = std::make_shared<std::vector<char>>(size);
        ifs.read(buffer->data(), static_cast<std::streamsize>(buffer->size()));
        FileCache::m_file_cache[path.string()] = buffer;
        SPDLOG_INFO("Loaded asset {} into cache", path.string());

        return buffer;
    } catch (std::system_error &e) {
        SPDLOG_CRITICAL("unable to open file {}: {}", path.string(), e.code().message().c_str());
    }

    return nullptr;
}

} // namespace ge
