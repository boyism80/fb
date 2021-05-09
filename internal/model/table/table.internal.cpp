#include "table.internal.h"

fb::internal::container::host       fb::internal::table::hosts;

std::optional<uint8_t> fb::internal::container::host::operator[](uint32_t id)
{
    auto found = std::map<uint32_t, uint8_t>::find(id);
    if(found == this->end())
        return std::nullopt;
    else
        return found->second;
}

bool fb::internal::container::host::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            auto                key = i.key().asString();
            auto                value = (*i).asUInt();

            this->insert(std::pair<uint32_t, uint8_t>(std::stoi(key), value));
            callback(key, percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                key = i.key().asString();
            error(key, e);
        }
        );

    complete(count);
    return true;
}