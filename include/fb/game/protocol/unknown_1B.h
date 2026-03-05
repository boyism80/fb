#ifndef __PROTOCOL_GAME_UNKNOWN_1B_H__
#define __PROTOCOL_GAME_UNKNOWN_1B_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class unknown_1B : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x1B;

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
