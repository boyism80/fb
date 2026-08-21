#include <fb/game/service/weather.h>
#include <fb/game/server.h>
#include <macro.h>
#include <utility>

using namespace fb::game;
using weather_type = fb::model::enum_value::WEATHER_TYPE;

service::weather::weather(fb::game::server& server) :
    server(server)
{ }

void service::weather::sync()
{
    const auto& time    = this->server.time();
    auto        day_key = static_cast<int>(time.year()) * 10000 + static_cast<int>(time.month()) * 100 +
                   static_cast<int>(time.day());
    if (day_key != this->_day_key)
    {
        this->_day_key = day_key;
        this->_segments.clear();

        auto seed = static_cast<uint32_t>(day_key);
        auto rng  = [&seed]() {
            seed = seed * 1103515245u + 12345u;
            return seed;
        };

        auto month = time.month();
        auto wN = 55u, wR = 45u, wS = 0u;
        if (month >= 6 && month <= 8)
        {
            wN = 40;
            wR = 60;
            wS = 0;
        }
        else if (month == 12 || month <= 2)
        {
            wN = 45;
            wR = 20;
            wS = 35;
        }
        else if (month >= 9 && month <= 11)
        {
            wN = 60;
            wR = 35;
            wS = 5;
        }

        auto    n            = 1u + (rng() % 4u);
        uint8_t hours[4]     = {};
        uint8_t hour_count   = 0;
        hours[hour_count++]  = 0;
        while (hour_count < n)
        {
            auto h   = static_cast<uint8_t>(rng() % 24);
            auto dup = false;
            for (uint8_t i = 0; i < hour_count; ++i)
            {
                if (hours[i] == h)
                {
                    dup = true;
                    break;
                }
            }
            if (dup)
                continue;
            hours[hour_count++] = h;
        }
        for (uint8_t i = 0; i < hour_count; ++i)
        {
            for (uint8_t j = i + 1; j < hour_count; ++j)
            {
                if (hours[j] < hours[i])
                    std::swap(hours[i], hours[j]);
            }
        }

        this->_segments.reserve(hour_count);
        for (uint8_t i = 0; i < hour_count; ++i)
        {
            auto         r = rng() % (wN + wR + wS);
            weather_type t = weather_type::NORMAL;
            if (r >= wN && r < wN + wR)
                t = weather_type::RAIN;
            else if (r >= wN + wR)
                t = weather_type::SNOW;
            this->_segments.push_back({hours[i], t});
        }
    }

    auto type = weather_type::NORMAL;
    auto hour = static_cast<uint8_t>(time.hours());
    for (const auto& segment : this->_segments)
    {
        if (segment.start_hour <= hour)
            type = segment.type;
        else
            break;
    }

    if (type == this->_current)
        return;

    this->_current = type;
    this->server.characters.foreach_enqueue([type](auto& ch) -> async::task<void> {
        auto map = ch->map();
        if (map == nullptr)
            co_return;
        if (ENUM_IN(map->model().option, fb::model::enum_value::MAP_OPTION::BUILD_IN))
            co_return;

        ch->weather(type);
        co_return;
    });
}

weather_type service::weather::current() const
{
    return this->_current;
}
