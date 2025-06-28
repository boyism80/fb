#ifndef __PROTOCOL_GAME_MIX_H__
#define __PROTOCOL_GAME_MIX_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class item_mix : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x6B;

public:
    std::vector<uint8_t> indices;

public:
    item_mix() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif