#ifndef __PROTOCOL_GAME_INACTIVE_H__
#define __PROTOCOL_GAME_INACTIVE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class item_inactive : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x1F;

public:
#ifndef BOT
    EQUIPMENT_PARTS parts;
#else
    const EQUIPMENT_PARTS parts;
#endif

public:
#ifndef BOT
    item_inactive() = default;
#else
    item_inactive(EQUIPMENT_PARTS parts) :
        parts(parts)
    { }
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif