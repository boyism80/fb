#ifndef __PROTOCOL_GAME_INFO_H__
#define __PROTOCOL_GAME_INFO_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class item_info : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x66;

public:
    uint16_t position;
    uint8_t  slot;

public:
    item_info() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif