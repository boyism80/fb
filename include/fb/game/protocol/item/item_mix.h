#ifndef __PROTOCOL_GAME_MIX_H__
#define __PROTOCOL_GAME_MIX_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class item_mix : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x6B;

public:
    std::vector<uint8_t> indices;

public:
    item_mix() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif