#ifndef __PROTOCOL_GAME_UNKNOWN_27_H__
#define __PROTOCOL_GAME_UNKNOWN_27_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

/**
 * C2S 0x27 — 5-row grid list UI slot click (client sub_4E6D30), since 5.65.
 * Wire: [0x27][0x00][0x01][slot u8] (4 bytes).
 * The owning screen (storage / mail / auction) is still unidentified.
 */
template <CLIENT_VERSION V>
class unknown_27 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x27;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565);

public:
#ifdef BOT
    const uint8_t slot;
#else
    uint8_t slot = 0;
#endif

public:
#ifdef BOT
    explicit unknown_27(uint8_t slot) :
        slot(slot)
    { }
#else
    unknown_27() = default;
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
