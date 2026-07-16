#ifndef __PROTOCOL_GAME_SCREEN_REFRESH_COMPLETE_H__
#define __PROTOCOL_GAME_SCREEN_REFRESH_COMPLETE_H__

#include <fb/protocol/header.h>

namespace fb::protocol::game::response {

class screen_refresh_complete : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x22;

public:
    screen_refresh_complete() = default;

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
