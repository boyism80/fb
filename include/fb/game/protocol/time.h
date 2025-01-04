#ifndef __PROTOCOL_GAME_TIME_H__
#define __PROTOCOL_GAME_TIME_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class time : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x20;

public:
#ifdef BOT
    uint8_t hours;
#else
    const uint8_t hours;
#endif

public:
#ifdef BOT
    time() = default;
#else
    time(uint8_t hours);
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif