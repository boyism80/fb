#ifndef __PROTOCOL_GAME_DROP_MONEY_H__
#define __PROTOCOL_GAME_DROP_MONEY_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_drop_money : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x24;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    uint32_t chunk;
#else
    const uint32_t chunk;
#endif

public:
#ifndef BOT
    item_drop_money() = default;
#else
    item_drop_money(uint32_t chunk);
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