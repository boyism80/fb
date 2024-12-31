#ifndef __PROTOCOL_GAME_FRONT_INFO_H__
#define __PROTOCOL_GAME_FRONT_INFO_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class front_info : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x09;

public:
    front_info() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif