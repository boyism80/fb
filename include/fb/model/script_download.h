#ifndef __FB_MODEL_SCRIPT_DOWNLOAD_H__
#define __FB_MODEL_SCRIPT_DOWNLOAD_H__

#include <fb/http_client.h>
#include <fb/logger.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fb::model {

inline std::string normalize_script_relative_path(std::string_view path)
{
    auto relative = std::string(path);
    for (auto& ch : relative)
    {
        if (ch == '\\')
            ch = '/';
    }
    while (!relative.empty() && relative.front() == '/')
        relative.erase(relative.begin());

    if (relative.empty())
        throw std::runtime_error("empty script path in ReloadScripts");
    if (relative.find("..") != std::string::npos)
        throw std::runtime_error(std::format("invalid script path: {}", relative));
    if (relative.starts_with("scripts/"))
        throw std::runtime_error(std::format("script path must not include scripts/ prefix: {}", relative));
    if (!relative.ends_with(".lua"))
        throw std::runtime_error(std::format("script path must end with .lua: {}", relative));

    return relative;
}

inline std::string join_script_download_url(std::string_view base_url, std::string_view relative)
{
    auto base = std::string(base_url);
    while (!base.empty() && base.back() == '/')
        base.pop_back();
    return std::format("{}/{}", base, relative);
}

/// Downloads Lua scripts into cwd-relative scripts/. All-or-nothing: writes only after every GET succeeds.
inline async::task<void> download_scripts(fb::http_client&                http,
                                          std::string_view                base_url,
                                          const std::vector<std::string>& script_paths)
{
    struct downloaded_file
    {
        std::string          relative;
        std::vector<uint8_t> bytes;
    };

    auto files = std::vector<downloaded_file>{};
    files.reserve(script_paths.size());

    for (const auto& name : script_paths)
    {
        auto relative = normalize_script_relative_path(name);
        auto url      = join_script_download_url(base_url, relative);
        auto bytes    = co_await http.get_bytes(url);
        if (bytes.empty())
            throw std::runtime_error(std::format("downloaded empty body: {}", url));

        files.push_back(downloaded_file{std::move(relative), std::move(bytes)});
    }

    auto temps = std::vector<std::filesystem::path>{};
    temps.reserve(files.size());

    try
    {
        for (const auto& file : files)
        {
            auto final_path = std::filesystem::path("scripts") / std::filesystem::path(file.relative);
            auto temp_path  = final_path;
            temp_path += ".tmp";

            std::filesystem::create_directories(final_path.parent_path());

            {
                auto out = std::ofstream(temp_path, std::ios::binary | std::ios::trunc);
                if (!out)
                    throw std::runtime_error(std::format("cannot write temp file: {}", temp_path.string()));
                out.write(reinterpret_cast<const char*>(file.bytes.data()),
                          static_cast<std::streamsize>(file.bytes.size()));
                if (!out)
                    throw std::runtime_error(std::format("failed writing temp file: {}", temp_path.string()));
            }

            temps.push_back(temp_path);
            std::filesystem::rename(temp_path, final_path);
        }
    }
    catch (...)
    {
        for (const auto& temp : temps)
        {
            std::error_code ec;
            std::filesystem::remove(temp, ec);
        }
        throw;
    }

    fb::logger::info("Downloaded {} script file(s) from {}", files.size(), base_url);
    co_return;
}

} // namespace fb::model

#endif
