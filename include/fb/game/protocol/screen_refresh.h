#ifndef __PROTOCOL_GAME_SCREEN_REFRESH_H__
#define __PROTOCOL_GAME_SCREEN_REFRESH_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class screen_refresh : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x38;

public:
    screen_refresh() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
