#ifndef __PROTOCOL_GAME_TIMER_H__
#define __PROTOCOL_GAME_TIMER_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class timer : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x67;

public:
#ifndef BOT
    const uint32_t   time;
    const TIMER_TYPE type;
#else
    uint32_t   time;
    TIMER_TYPE type;
#endif

public:
#ifndef BOT
    timer(uint32_t time, TIMER_TYPE type = TIMER_TYPE::DECREASE) :
        time(time),
        type(type)
    { }
#else
    timer() = default;
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