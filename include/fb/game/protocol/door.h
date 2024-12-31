#ifndef __PROTOCOL_GAME_DOOR_H__
#define __PROTOCOL_GAME_DOOR_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class door : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x20;

public:
    door() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif