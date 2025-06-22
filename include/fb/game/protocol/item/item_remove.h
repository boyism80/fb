#ifndef __PROTOCOL_GAME_ITEM_REMOVE_H__
#define __PROTOCOL_GAME_ITEM_REMOVE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class item_remove : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x10;

public:
#ifndef BOT
    const ITEM_DELETE_TYPE type;
    const uint32_t         index;
    const uint16_t         count;
#else
    ITEM_DELETE_TYPE type;
    uint32_t         index;
    uint16_t         count;
#endif

public:
#ifndef BOT
    item_remove(ITEM_DELETE_TYPE type, uint32_t index, uint16_t count = 0);
#else
    item_remove() = default;
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