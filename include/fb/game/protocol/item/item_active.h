#ifndef __PROTOCOL_GAME_ACTIVE_H__
#define __PROTOCOL_GAME_ACTIVE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class item_active : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x1C;

public:
    uint8_t index;

public:
    item_active() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif