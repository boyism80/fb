#ifndef __PROTOCOL_GAME_INIT_H__
#define __PROTOCOL_GAME_INIT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class init : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x1E;

public:
    init() = default;

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#ifdef BOT
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif