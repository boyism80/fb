#ifndef __PROTOCOL_GATEWAY_ASSERT_VERSION_H__
#define __PROTOCOL_GATEWAY_ASSERT_VERSION_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>

namespace fb::protocol::gateway::request {

/**
 * C2S version (opcode 0x00).
 * Primary layout (v550): u16 version + u8 nation.
 * v565/v651: + u16 build after nation (if constexpr).
 *
 * Bootstrap (no session yet) always deserializes as version<v550>; the packed
 * version field still establishes CLIENT_VERSION for the session.
 */
template <CLIENT_VERSION V>
class version : public fb::protocol::header
{
public:
    // A member cannot share the enclosing class name, so the version tag is
    // named `protocol_version` here; the handler registry accepts either name.
    static constexpr uint8_t opcode = 0x00;
    FB_PROTOCOL_VERSION_TAGS_NAMED(V);

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
