#ifndef __PROTOCOL_GAME_SELF_INFO_H__
#define __PROTOCOL_GAME_SELF_INFO_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class self_info : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2D;

public:
    self_info() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif