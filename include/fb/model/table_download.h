#ifndef __FB_MODEL_TABLE_DOWNLOAD_H__
#define __FB_MODEL_TABLE_DOWNLOAD_H__

#include <fb/http_client.h>
#include <fb/logger.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace fb::model {

inline std::string table_file_stem(std::string_view name)
{
    auto stem = std::string(name);
    if (stem.size() >= 5 && stem.ends_with(".json"))
        stem.resize(stem.size() - 5);
    return stem;
}

inline std::string join_download_url(std::string_view base_url, std::string_view stem)
{
    auto base = std::string(base_url);
    while (!base.empty() && base.back() == '/')
        base.pop_back();
    return std::format("{}/{}.json", base, stem);
}

/// Downloads table JSON files into cwd-relative json/. All-or-nothing: writes only after every GET succeeds.
inline async::task<void> download_tables(fb::http_client&                  http,
                                         std::string_view                  base_url,
                                         const std::vector<std::string>&   table_names)
{
    struct downloaded_file
    {
        std::string              stem;
        std::vector<uint8_t>     bytes;
    };

    auto files = std::vector<downloaded_file>{};
    files.reserve(table_names.size());

    for (const auto& name : table_names)
    {
        auto stem = table_file_stem(name);
        if (stem.empty())
            throw std::runtime_error("empty table name in ReloadTables");

        auto url  = join_download_url(base_url, stem);
        auto bytes = co_await http.get_bytes(url);
        if (bytes.empty())
            throw std::runtime_error(std::format("downloaded empty body: {}", url));

        files.push_back(downloaded_file{std::move(stem), std::move(bytes)});
    }

    auto json_dir = std::filesystem::path("json");
    std::filesystem::create_directories(json_dir);

    auto temps = std::vector<std::filesystem::path>{};
    temps.reserve(files.size());

    try
    {
        for (const auto& file : files)
        {
            auto final_path = json_dir / std::format("{}.json", file.stem);
            auto temp_path  = json_dir / std::format("{}.json.tmp", file.stem);

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

    fb::logger::info("Downloaded {} table file(s) from {}", files.size(), base_url);
    co_return;
}

} // namespace fb::model

#endif
