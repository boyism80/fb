#ifndef __PROTOCOL_GAME_OBJECT_MISS_H__
#define __PROTOCOL_GAME_OBJECT_MISS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class miss : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0C;

public:
    uint32_t oid;

public:
    miss() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif