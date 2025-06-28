#ifndef __PROTOCOL_GAME_SWAP_H__
#define __PROTOCOL_GAME_SWAP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class swap : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x30;

public:
    SWAP_TYPE type;
    uint8_t   src;
    uint8_t   dst;

public:
    swap() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif