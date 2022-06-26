#include <fb/internal/data_set.h>

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
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                id = key.asString();
            auto                value = data.asUInt();

            this->insert(std::pair<uint32_t, uint8_t>(std::stoi(id), value));
            callback(id, percentage);
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            error(key.asString(), e);
        },
        false
    );

    complete(count);
    return true;
}