#ifndef __PROTOCOL_GAME_OBJECT_MISS_H__
#define __PROTOCOL_GAME_OBJECT_MISS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class miss : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x0C;

public:
    uint32_t sequence;

public:
    miss() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif