#ifndef __PROTOCOL_GAME_UPDATE_MOVE_H__
#define __PROTOCOL_GAME_UPDATE_MOVE_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <fb/game/protocol/object/move.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class update_move : public move<V>
{
public:
    static constexpr uint8_t opcode = 0x06;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    fb::model::point<uint16_t> begin;
    fb::model::size<uint8_t>   size;
    uint16_t                   crc;

public:
    update_move() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif