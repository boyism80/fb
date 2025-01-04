#ifndef __PROTOCOL_GAME_PICK_UP_H__
#define __PROTOCOL_GAME_PICK_UP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class pick_up : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x07;

public:
#ifndef BOT
    bool boost;
#else
    const bool boost;
#endif

public:
#ifndef BOT
    pick_up() = default;
#else
    pick_up(bool boost);
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