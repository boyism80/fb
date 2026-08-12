#ifndef __PROTOCOL_GAME_PICK_UP_H__
#define __PROTOCOL_GAME_PICK_UP_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class loot : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x07;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    bool boost;
#else
    const bool boost;
#endif

public:
#ifndef BOT
    loot() = default;
#else
    loot(bool boost);
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