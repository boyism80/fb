#ifndef __PROTOCOL_GAME_ITEM_UPDATE_H__
#define __PROTOCOL_GAME_ITEM_UPDATE_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_update : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0F;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const uint8_t     index;
    const uint16_t    look;
    const uint8_t     color;
    const std::string name;
    const uint32_t    count;
    const bool        valid;
#else
    uint8_t     index;
    uint16_t    look;
    uint8_t     color;
    std::string name;
    uint32_t    count;
#endif

public:
#ifndef BOT
    item_update(const fb::game::character& me, uint8_t index);
    item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count);
#else
    item_update() = default;
#endif

private:
#ifndef BOT
    item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count, bool valid);
    static item_update from_inventory(const fb::game::character& me, uint8_t index);
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <>
class item_update<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0F;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    const uint8_t     index;
    const uint16_t    look;
    const uint8_t     color;
    const std::string name;
    const uint32_t    count;
    const uint8_t     flag;
    const uint32_t    extra;
    const bool        valid;
#else
    uint8_t     index;
    uint16_t    look;
    uint8_t     color;
    std::string name;
    std::string name_b;
    uint32_t    count;
    uint8_t     flag  = 0;
    uint32_t    extra = 0;
#endif

public:
#ifndef BOT
    item_update(const fb::game::character& me, uint8_t index);
    item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count);
#else
    item_update() = default;
#endif

private:
#ifndef BOT
    item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count, bool valid);
    static item_update from_inventory(const fb::game::character& me, uint8_t index);
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

using item_update_v550 = item_update<CLIENT_VERSION::v550>;
using item_update_v565 = item_update<CLIENT_VERSION::v565>;
using item_update_v651 = item_update<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
