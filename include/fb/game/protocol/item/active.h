#ifndef __PROTOCOL_GAME_ACTIVE_H__
#define __PROTOCOL_GAME_ACTIVE_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_active : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x1C;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    uint8_t index;

public:
#ifndef BOT
    item_active() = default;
#else
    item_active(uint8_t index);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif