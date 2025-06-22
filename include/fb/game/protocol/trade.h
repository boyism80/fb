#ifndef __PROTOCOL_GAME_TRADE_H__
#define __PROTOCOL_GAME_TRADE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/trade.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class trade : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x4A;

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
    uint8_t  action;
    uint32_t fd;
    params   parameter;
#else
    const uint8_t  action;
    const uint32_t fd;
    const params   parameter;
#endif

public:
    trade() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif