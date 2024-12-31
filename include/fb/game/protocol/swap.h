#ifndef __PROTOCOL_GAME_SWAP_H__
#define __PROTOCOL_GAME_SWAP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class swap : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x30;

public:
    SWAP_TYPE type;
    uint8_t   src;
    uint8_t   dst;

public:
    swap() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif