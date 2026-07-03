#ifndef __PROTOCOL_GAME_MONEY_H__
#define __PROTOCOL_GAME_MONEY_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class trade_money : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x42;

public:
#ifndef BOT
    const uint32_t money;
    const bool     mine;
#else
    uint32_t money;
    bool     mine;
#endif

public:
#ifndef BOT
    trade_money(uint32_t money, bool mine);
#else
    trade_money() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif