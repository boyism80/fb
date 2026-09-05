#ifndef __PROTOCOL_GAME_GROUP_PORTRAIT_H__
#define __PROTOCOL_GAME_GROUP_PORTRAIT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>
#include <vector>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

struct group_portrait_entry
{
    std::string name;
    uint8_t     leader          = 0;
    uint16_t    hair            = 0;
    uint8_t     color           = 0;
    uint8_t     face_hair_tint  = 0;
    uint8_t     hair_to_hat     = 0;
    uint8_t     helmet          = 0;
    uint8_t     helmet_color    = 0;
    uint16_t    accessory_pack  = 0xFFFF;
    uint8_t     accessory_color = 0;
    uint32_t    max_hp          = 0;
    uint32_t    cur_hp          = 0;
};

template <CLIENT_VERSION V>
class group_portrait : public fb::protocol::header
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
    group_portrait() = default;
#else
    group_portrait(uint8_t subtype, uint8_t count);
#endif

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

template <>
class group_portrait<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x63;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifdef BOT
    uint8_t                           subtype = 2;
    uint8_t                           count   = 0;
    std::vector<group_portrait_entry> entries;
#else
    const uint8_t                           subtype;
    const uint8_t                           count;
    const std::vector<group_portrait_entry> entries;
#endif

public:
#ifdef BOT
    group_portrait() = default;
#else
    group_portrait(uint8_t subtype, uint8_t count, std::vector<group_portrait_entry> entries = {});
    group_portrait(std::string name, uint32_t cur_hp);
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
