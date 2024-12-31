#ifndef __PROTOCOL_GAME_WEATHER_H__
#define __PROTOCOL_GAME_WEATHER_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class weather : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x1F;

public:
    const WEATHER_TYPE value;

public:
    weather(WEATHER_TYPE value);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif