#ifndef __PROTOCOL_GAME_BUFF_H__
#define __PROTOCOL_GAME_BUFF_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/spell.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class spell_buff : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x3A;

public:
#ifndef BOT
    const std::string               name;
    const std::chrono::milliseconds time;
#else

#endif

public:
#ifndef BOT
    spell_buff(const std::string& name, uint32_t time);
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