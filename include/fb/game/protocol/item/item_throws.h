#ifndef __PROTOCOL_GAME_ITEM_THROWS_H__
#define __PROTOCOL_GAME_ITEM_THROWS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/item.h>
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;
using namespace fb::game;

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
    const character&                 ch;
    const item&                      item;
    const fb::model::point<uint16_t> to;
#else

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