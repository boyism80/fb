#ifndef __PROTOCOL_GAME_UNKNOWN_12_H__
#define __PROTOCOL_GAME_UNKNOWN_12_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x12 — purpose still unclear (not a group/party packet).
 * Confirmed wire fields:
 * - oid: local player only on CharStats path
 * - slot: inventory letter index (1='a'..); appends str.res 148 "[무장]" when flag < 0xA0
 * - flag: branch gate (< 0xA0 armed-mark path, >= 0xA0 self level-up toast via template 14)
 */
template <CLIENT_VERSION V>
class unknown_12 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x12;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const uint32_t oid;
    const uint8_t  slot;
    const uint8_t  flag;
#else
    uint32_t oid  = 0;
    uint8_t  slot = 0;
    uint8_t  flag = 0;
#endif

public:
#ifndef BOT
    unknown_12(uint32_t oid, uint8_t slot, uint8_t flag) :
        oid(oid),
        slot(slot),
        flag(flag)
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
