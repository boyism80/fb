#ifndef __PROTOCOL_GAME_TRADE_H__
#define __PROTOCOL_GAME_TRADE_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/trade.h>
#endif

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class trade : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x4A;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    struct params
    {
        uint8_t  index = 0;
        uint16_t count = 0;
        uint32_t money = 0;
    };

public:
#ifndef BOT
    fb::game::trade::state action;
    uint32_t               oid;
    params                 parameter;
#else
    enum class state : uint8_t
    {
        REQUEST    = 0x00,
        UP_ITEM    = 0x01,
        ITEM_COUNT = 0x02,
        UP_MONEY   = 0x03,
        CANCEL     = 0x04,
        LOCK       = 0x05,
    };

    const state    action;
    const uint32_t oid;
    const params   parameter;
#endif

public:
#ifndef BOT
    trade() = default;
#else
    trade(state action, uint32_t oid, const params& parameter);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

#ifdef BOT
template <>
void trade<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#else
template <>
void trade<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader);
#endif

} // namespace fb::protocol::game::request

#endif