#ifndef __FB_GAME_SERVICE_WEATHER_H__
#define __FB_GAME_SERVICE_WEATHER_H__

#include <fb/model/model.h>
#include <vector>
#include <cstdint>

namespace fb::game {
class server;
} // namespace fb::game

namespace fb::game::service {

class weather
{
public:
    fb::game::server& server;

private:
    struct segment
    {
        uint8_t                             start_hour = 0;
        fb::model::enum_value::WEATHER_TYPE type       = fb::model::enum_value::WEATHER_TYPE::NORMAL;
    };

    fb::model::enum_value::WEATHER_TYPE _current = fb::model::enum_value::WEATHER_TYPE::NORMAL;
    int                                 _day_key = -1;
    std::vector<segment>                _segments;

    void                                rebuild_segments(uint32_t seed, int month, int day);
    fb::model::enum_value::WEATHER_TYPE type_at_hour(uint8_t hour) const;
    void                                broadcast_outdoor(fb::model::enum_value::WEATHER_TYPE type);

public:
    explicit weather(fb::game::server& server);

    void                                sync();
    fb::model::enum_value::WEATHER_TYPE reroll(uint32_t max_attempts = 64);
    fb::model::enum_value::WEATHER_TYPE current() const;
};

} // namespace fb::game::service

#endif
