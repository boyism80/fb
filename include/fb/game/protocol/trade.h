#ifndef __PROTOCOL_GAME_TRADE_H__
#define __PROTOCOL_GAME_TRADE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/trade.h>
#endif

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class trade : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x4A;

public:
    typedef union
    {
    public:
        uint8_t  index;
        uint16_t count;
        uint32_t money;
    } params;

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
    trade(state action, uint32_t oid, const params& parameter) :
        action(action),
        oid(oid),
        parameter(parameter)
    { }
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif