#include <fb/bot/load/trace.h>

#include <fb/encoding.h>

#include <chrono>
#include <filesystem>
#include <format>
#include <mutex>
#include <string>

using namespace fb::bot::load;

void trace::open(const std::string& path)
{
    auto lock = std::lock_guard(this->_mutex);
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    this->_file.open(path, std::ios::app);
}

void trace::write(const std::string& bot, Json::Value entry)
{
    entry["t"]   = std::format("{}", std::chrono::system_clock::now());
    entry["bot"] = fb::UTF8(bot, fb::PLATFORM::WINDOWS);

    auto writer           = Json::StreamWriterBuilder{};
    writer["indentation"] = "";
    writer["emitUTF8"]    = true;
    auto line             = Json::writeString(writer, entry);

    auto lock = std::lock_guard(this->_mutex);
    if (this->_file.is_open())
        this->_file << line << '\n' << std::flush;
}
