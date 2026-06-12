#ifndef __FB_GAME_SERVICE_PROPERTY_H__
#define __FB_GAME_SERVICE_PROPERTY_H__

#include <fb/synchronized.h>
#include <json/json.h>
#include <string>
#include <unordered_map>

namespace fb::game::service {

class property
{
private:
    using storage = std::unordered_map<std::string, Json::Value>;

    fb::synchronized<storage> _values;

public:
    auto enter_read() const
    {
        return _values.enter_read();
    }

    auto enter_write()
    {
        return _values.enter_write();
    }
};

} // namespace fb::game::service

#endif
