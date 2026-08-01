#ifndef __PROTOCOL_GAME_ITEM_THROWS_H__
#define __PROTOCOL_GAME_ITEM_THROWS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/item.h>
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::request {
#ifndef BOT
using namespace fb::game;
#endif
using namespace fb::model::enum_value;

class item_throws : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x17;

public:
#ifndef BOT
    bool    all;
    uint8_t index;
#else
    const bool    all;
    const uint8_t index;
#endif

public:
#ifndef BOT
    item_throws() = default;
#else
    item_throws(const bool all, const uint8_t index) :
        all(all),
        index(index)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

class item_throws : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x16;

public:
#ifndef BOT
    const fb::game::character&       ch;
    const fb::game::item&            item;
    const fb::model::point<uint16_t> to;
#else
    uint32_t ch_oid;
    uint16_t look;
    uint8_t  color;
    uint32_t item_oid;
    uint16_t from_x;
    uint16_t from_y;
    uint16_t to_x;
    uint16_t to_y;
#endif

public:
#ifndef BOT
    item_throws(const fb::game::character& ch, const fb::game::item& item, const fb::model::point<uint16_t>& to);
#else
    item_throws() = default;
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
