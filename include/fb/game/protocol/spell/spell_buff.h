#ifndef __PROTOCOL_GAME_BUFF_H__
#define __PROTOCOL_GAME_BUFF_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/spell.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class spell_buff : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x3A;

public:
#ifndef BOT
    const std::string         name;
    const fb::model::timespan duration;
#else
    std::string         name;
    fb::model::timespan duration;
#endif

public:
#ifndef BOT
    spell_buff(std::string_view name, const fb::model::timespan& duration);
    spell_buff(const fb::game::buff& buff);
#else
    spell_buff() = default;
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