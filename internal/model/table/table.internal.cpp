#include "table.internal.h"

fb::internal::container::host       fb::internal::table::hosts;

std::string* fb::internal::container::host::operator[](uint32_t id)
{
    auto found = std::map<uint32_t, std::string>::find(id);
    if(found == this->end())
        return nullptr;
    else
        return &(found->second);
}

bool fb::internal::container::host::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            auto                key = i.key().asString();
            auto                value = (*i).asString();

            this->insert(std::pair<uint32_t, std::string>(std::stoi(key), value));
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