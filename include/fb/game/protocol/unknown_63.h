#ifndef __PROTOCOL_GAME_UNKNOWN_63_H__
#define __PROTOCOL_GAME_UNKNOWN_63_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x63 — SELFLOOK appearance list (CharStats NEW UI only), since 6.51.
 * Only subtype 2 is consumed by the client: [0x63][subtype u8][count u8] then
 * count entries of stride 280. Entries are not modelled; count 0 opens an empty list.
 */
template <CLIENT_VERSION V>
class unknown_63 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x63;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
#ifdef BOT
    uint8_t subtype = 2;
    uint8_t count   = 0;
#else
    const uint8_t subtype;
    const uint8_t count;
#endif

public:
#ifdef BOT
    unknown_63() = default;
#else
    unknown_63(uint8_t subtype, uint8_t count) :
        subtype(subtype),
        count(count)
    { }
#endif

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

} // namespace fb::protocol::game::response

#endif
