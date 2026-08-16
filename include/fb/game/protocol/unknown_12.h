#ifndef __PROTOCOL_GAME_UNKNOWN_12_H__
#define __PROTOCOL_GAME_UNKNOWN_12_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <vector>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

struct unknown_12_entry
{
    uint8_t id    = 0;
    uint8_t extra = 0;
};

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
    unknown_12(uint32_t oid, uint8_t slot, uint8_t flag);
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

template <>
class unknown_12<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x12;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    const uint8_t                       type;
    const std::vector<unknown_12_entry> entries;
    const uint8_t                       group_id;
    const std::vector<uint8_t>          bitmask;
    const uint8_t                       id;
    const uint8_t                       slot;
    const uint8_t                       onoff;
#else
    uint8_t                       type = 2;
    std::vector<unknown_12_entry> entries;
    uint8_t                       group_id = 0;
    std::vector<uint8_t>          bitmask;
    uint8_t                       id    = 0;
    uint8_t                       slot  = 0;
    uint8_t                       onoff = 0;
#endif

public:
#ifndef BOT
    unknown_12(uint32_t oid, uint8_t slot, uint8_t flag);
    unknown_12(uint8_t type, uint8_t id, uint8_t slot, uint8_t onoff);
    explicit unknown_12(std::vector<unknown_12_entry> entries);
    unknown_12(uint8_t group_id, std::vector<uint8_t> bitmask);
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
