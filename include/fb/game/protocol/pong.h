#ifndef __PROTOCOL_GAME_PONG_H__
#define __PROTOCOL_GAME_PONG_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

class pong : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x75;

public:
#ifndef BOT
    uint32_t token          = 0;
    uint32_t client_tick_ms = 0; // client timeGetTime(); server ignores
#else
    const uint32_t token;
    const uint32_t client_tick_ms;
#endif

public:
#ifndef BOT
    pong() = default;
#else
    pong(uint32_t token, uint32_t client_tick_ms) :
        token(token),
        client_tick_ms(client_tick_ms)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
