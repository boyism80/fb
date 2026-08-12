#ifndef __PROTOCOL_GAME_UNKNOWN_6F_H__
#define __PROTOCOL_GAME_UNKNOWN_6F_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x6F — name list sync (client sub_4A8100), since 5.65.
 * subtype 0 replaces a single entry, subtype 1 carries a u16 entry count.
 * Only the subtype 1 header is modelled; an empty list (count 0) is a safe probe.
 */
template <CLIENT_VERSION V>
class unknown_6f : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x6F;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565);

public:
#ifdef BOT
    uint8_t  subtype = 1;
    uint16_t count   = 0;
#else
    const uint8_t  subtype;
    const uint16_t count;
#endif

public:
#ifdef BOT
    unknown_6f() = default;
#else
    unknown_6f(uint8_t subtype, uint16_t count) :
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
