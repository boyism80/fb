#ifndef __PROTOCOL_GATEWAY_CRT_H__
#define __PROTOCOL_GATEWAY_CRT_H__

#include <fb/protocol/header.h>
#include <fb/crypto.h>

namespace fb::protocol::gateway::response {

class crypto : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x00;

public:
#ifdef BOT
    fb::crypto crt;
    uint32_t   crc;
#else
    const fb::crypto crt;
    const uint32_t   crc;
#endif

public:
#ifdef BOT
    crypto() = default;
#else
    crypto(const fb::crypto& crypto, uint32_t crc);
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::gateway::response

#endif