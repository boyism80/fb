#ifndef __PROTOCOL_GAME_ITEM_UPDATE_H__
#define __PROTOCOL_GAME_ITEM_UPDATE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class item_update : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x0F;

public:
#ifndef BOT
    const fb::game::character& me;
    const uint8_t              index;
#else

#endif

public:
#ifndef BOT
    item_update(const fb::game::character& me, uint8_t index);
#else

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