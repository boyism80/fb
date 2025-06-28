#ifndef __PROTOCOL_GAME_TRADE_DIALOG_H__
#define __PROTOCOL_GAME_TRADE_DIALOG_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class trade_dialog : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x42;

public:
#ifndef BOT
    const fb::game::character& me;
    const fb::model::model&    model;
#else
    uint32_t    sequence;
    std::string name;
#endif

public:
#ifndef BOT
    trade_dialog(const fb::game::character& me, const fb::model::model& model);
#else
    trade_dialog() = default;
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