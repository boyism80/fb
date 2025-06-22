#ifndef __PROTOCOL_GAME_ITEM_THROWS_H__
#define __PROTOCOL_GAME_ITEM_THROWS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/item.h>
#include <fb/game/character.h>
#endif

#ifndef BOT
using namespace fb::game;
#endif
using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class item_throws : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x17;

public:
    bool    all;
    uint8_t index;

public:
    item_throws() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

class item_throws : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x16;

public:
#ifndef BOT
    const fb::game::character&       ch;
    const fb::game::item&            item;
    const fb::model::point<uint16_t> to;
#else
    uint32_t ch_sequence;
    uint16_t look;
    uint8_t  color;
    uint32_t item_sequence;
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
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif