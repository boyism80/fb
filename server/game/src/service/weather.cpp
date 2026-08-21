#include <fb/game/service/weather.h>
#include <fb/game/server.h>
#include <macro.h>
#include <utility>

using namespace fb::game;
using weather_type = fb::model::enum_value::WEATHER_TYPE;

namespace {

struct weather_weights
{
    uint32_t normal = 0;
    uint32_t rain   = 0;
    uint32_t snow   = 0;
};

// Approximate Seoul / central Korea climate:
// - Wet-day rates from monthly normals (most rain in late Jun-Jul jangma, then Aug)
// - Snow almost only Dec-Feb; none in spring through autumn lowlands
// - Jangma roughly Jun 25 - Jul 26 (KMA central-region normal)
bool in_jangma(int month, int day)
{
    if (month == 6 && day >= 25)
        return true;
    if (month == 7 && day <= 26)
        return true;
    return false;
}

weather_weights weights_for(int month, int day)
{
    switch (month)
    {
    case 1:
        return {90, 6, 4};
    case 2:
        return {90, 7, 3};
    case 3:
        return {85, 15, 0};
    case 4:
        return {80, 20, 0};
    case 5:
        return {78, 22, 0};
    case 6:
        if (day >= 25)
            return {50, 50, 0};
        return {72, 28, 0};
    case 7:
        if (day <= 26)
            return {45, 55, 0};
        return {55, 45, 0};
    case 8:
        return {55, 45, 0};
    case 9:
        return {75, 25, 0};
    case 10:
        return {85, 15, 0};
    case 11:
        return {85, 15, 0};
    case 12:
        return {88, 7, 5};
    default:
        return {85, 15, 0};
    }
}

// Fewer shifts in dry seasons; more shower-like changes in wet months / jangma.
uint32_t segment_count(int month, int day, auto& rng)
{
    uint32_t max_extra = 1;
    if (in_jangma(month, day) || month == 8)
        max_extra = 3;
    else if (month >= 5 && month <= 9)
        max_extra = 2;
    else if (month >= 3 && month <= 4)
        max_extra = 2;

    return 1u + (rng() % (max_extra + 1u));
}

} // namespace

service::weather::weather(fb::game::server& server) :
    server(server)
{ }

void service::weather::sync()
{
    const auto& time = this->server.time();
    auto        day_key =
        static_cast<int>(time.year()) * 10000 + static_cast<int>(time.month()) * 100 + static_cast<int>(time.day());
    if (day_key != this->_day_key)
    {
        this->_day_key = day_key;
        this->_segments.clear();

        auto seed = static_cast<uint32_t>(day_key);
        auto rng  = [&seed]() {
            seed = seed * 1103515245u + 12345u;
            return seed;
        };

        auto month = static_cast<int>(time.month());
        auto day   = static_cast<int>(time.day());
        auto w     = weights_for(month, day);
        auto n     = segment_count(month, day, rng);

        uint8_t hours[4]    = {};
        uint8_t hour_count  = 0;
        hours[hour_count++] = 0;
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

        auto total = w.normal + w.rain + w.snow;
        this->_segments.reserve(hour_count);
        for (uint8_t i = 0; i < hour_count; ++i)
        {
            auto         r = rng() % total;
            weather_type t = weather_type::NORMAL;
            if (r >= w.normal && r < w.normal + w.rain)
                t = weather_type::RAIN;
            else if (r >= w.normal + w.rain)
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
