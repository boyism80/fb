#ifndef __PROTOCOL_GAME_TIMER_H__
#define __PROTOCOL_GAME_TIMER_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class timer : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x67;

public:
    const uint32_t   time;
    const TIMER_TYPE type;

public:
    timer(uint32_t time, TIMER_TYPE type = TIMER_TYPE::DECREASE);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif