#ifndef __PROTOCOL_GAME_UNKNOWN_12_H__
#define __PROTOCOL_GAME_UNKNOWN_12_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class unknown_12 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x12;

public:
#ifndef BOT
    const uint32_t oid;
    const uint8_t  party_slot;
    const uint8_t  level_encoded;
#else
    uint32_t oid;
    uint8_t  party_slot;
    uint8_t  level_encoded;
#endif

public:
#ifndef BOT
    unknown_12(uint32_t oid, uint8_t party_slot, uint8_t level_encoded) :
        oid(oid),
        party_slot(party_slot),
        level_encoded(level_encoded)
    { }
#else
    unknown_12() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
