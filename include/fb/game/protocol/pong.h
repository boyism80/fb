#ifndef __PROTOCOL_GAME_PONG_H__
#define __PROTOCOL_GAME_PONG_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

/**
 * Client-to-server pong (0x75).
 * Sent by the client after receiving server ping (0x68).
 * Wire (after cmd): token (4 bytes BE), unknown (4 bytes BE). Total 9 bytes.
 */
class pong : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x75;

public:
#ifndef BOT
    uint32_t token   = 0;
    uint32_t unknown = 0;
#else
    const uint32_t token;
    const uint32_t unknown;
#endif

public:
#ifndef BOT
    pong() = default;
#else
    pong(uint32_t token, uint32_t unknown) :
        token(token),
        unknown(unknown)
    { }
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
