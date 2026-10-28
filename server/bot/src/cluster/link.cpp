#include <fb/bot/cluster/link.h>
#include <fb/console.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>

using namespace fb::bot::cluster;

bool link::enabled() const
{
    return this->_enabled;
}

void link::enable()
{
    if (this->_enabled.exchange(true))
        return;

    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::thread([this]() {
        auto line = std::string{};
        while (std::getline(std::cin, line))
        {
            auto message = Json::Value{};
            if (link::parse(line, message) == false)
                continue;

            auto fn = handler{};
            {
                auto lock  = std::lock_guard(this->_mutex);
                auto found = this->_handlers.find(message["type"].asString());
                if (found != this->_handlers.end())
                    fn = found->second;
            }

            if (fn)
                fn(message);
        }

        auto fn = handler{};
        {
            auto lock  = std::lock_guard(this->_mutex);
            auto found = this->_handlers.find("stop");
            if (found != this->_handlers.end())
                fn = found->second;
        }

        if (fn)
            fn(Json::Value{});
    }).detach();
}

void link::on(const std::string& type, handler fn)
{
    auto lock = std::lock_guard(this->_mutex);
    this->_handlers.insert_or_assign(type, std::move(fn));
}

void link::send(const Json::Value& message) const
{
    if (this->_enabled == false)
        return;

    fb::console::puts("{}{}", MARKER, link::serialize(message));
}

std::string link::serialize(const Json::Value& message)
{
    auto writer           = Json::StreamWriterBuilder{};
    writer["indentation"] = "";
    writer["emitUTF8"]    = true;
    return Json::writeString(writer, message);
}

bool link::parse(std::string_view text, Json::Value& message)
{
    auto begin = text.find(MARKER);
    if (begin == std::string_view::npos)
        return false;

    text     = text.substr(begin + MARKER.size());
    auto end = text.rfind('}');
    if (end == std::string_view::npos)
        return false;

    return link::decode(text.substr(0, end + 1), message);
}

bool link::decode(std::string_view text, Json::Value& message)
{
    auto reader = std::unique_ptr<Json::CharReader>(Json::CharReaderBuilder{}.newCharReader());
    auto errors = std::string{};
    return reader->parse(text.data(), text.data() + text.size(), &message, &errors) && message.isObject();
}
