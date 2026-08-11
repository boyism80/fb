#ifndef __PROTOCOL_GATEWAY_ASSERT_VERSION_H__
#define __PROTOCOL_GATEWAY_ASSERT_VERSION_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>

namespace fb::protocol::gateway::request {

/**
 * C2S version (opcode 0x00).
 * Wire layout depends on CLIENT_VERSION discovered from the packed version field:
 *   v550: u16 version + u8 nation
 *   v565: u16 version + u8 nation + u16 build
 *
 * Bound as a single concrete type because the opcode is shared and V is only
 * known after reading the first field. Bot serialize selects layout from
 * client_version / build.
 */
class version : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x00;

public:
    uint16_t       v              = 0;
    uint8_t        nation         = 0;
    uint16_t       build          = 0;
    CLIENT_VERSION client_version = CLIENT_VERSION::v550;

public:
    version() = default;
#ifdef BOT
    version(CLIENT_VERSION client_version, uint8_t nation, uint16_t build = 0);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::gateway::request

#endif
