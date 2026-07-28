#ifndef __PROTOCOL_GAME_TIME_H__
#define __PROTOCOL_GAME_TIME_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class time : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x20;

public:
#ifndef BOT
    const uint8_t hours;
    const uint8_t minutes;
#else
    uint8_t hours;
    uint8_t minutes;
#endif

public:
#ifndef BOT
    time(uint8_t hours, uint8_t minutes = 0) :
        hours(hours),
        minutes(minutes)
    { }
#else
    time() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
