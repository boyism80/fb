#ifndef __PROTOCOL_GAME_UNKNOWN_54_H__
#define __PROTOCOL_GAME_UNKNOWN_54_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

/**
 * C2S 0x54 — round trip of S2C 0x4F UI window, since 5.65.
 * Wire header: [0x54][0x01][token u32 BE][phase u8], followed by a phase specific
 * payload. phase 5 is the window creation ack, phases 0..4 are user actions.
 * The trailing payload is left unread; the acceptor rewinds to the packet size.
 */
template <CLIENT_VERSION V>
class unknown_54 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x54;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565);

public:
#ifdef BOT
    const uint32_t token;
    const uint8_t  phase;
#else
    uint32_t token = 0;
    uint8_t  phase = 0;
#endif

public:
#ifdef BOT
    unknown_54(uint32_t token, uint8_t phase) :
        token(token),
        phase(phase)
    { }
#else
    unknown_54() = default;
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
