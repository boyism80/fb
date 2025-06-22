#ifndef __PROTOCOL_GAME_HIDE_H__
#define __PROTOCOL_GAME_HIDE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class hide : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x0E;

public:
#ifndef BOT
    const uint32_t id;
#else
    uint32_t id;
#endif

public:
#ifndef BOT
    hide(const fb::game::object& object);
    hide(uint32_t id);
#else
    hide() = default;
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