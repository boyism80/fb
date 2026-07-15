#ifndef __PROTOCOL_GAME_TRADE_DIALOG_H__
#define __PROTOCOL_GAME_TRADE_DIALOG_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class trade_dialog : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x42;

public:
#ifndef BOT
    const fb::game::character& me;
#else
    uint32_t    oid;
    std::string name;
#endif

public:
#ifndef BOT
    trade_dialog(const fb::game::character& me);
#else
    trade_dialog() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif