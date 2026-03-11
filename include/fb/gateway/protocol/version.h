#ifndef __PROTOCOL_GATEWAY_ASSERT_VERSION_H__
#define __PROTOCOL_GATEWAY_ASSERT_VERSION_H__

#include <fb/protocol/header.h>

namespace fb::protocol::gateway::request {

class version : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x00;

public:
    uint16_t v;
    uint8_t  nation;

public:
    version() = default;
#ifdef BOT
    version(uint16_t v, uint8_t nation);
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::gateway::request

#endif