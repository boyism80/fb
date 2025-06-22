#ifndef __PROTOCOL_GAME_CHARACTER_SHOW_H__
#define __PROTOCOL_GAME_CHARACTER_SHOW_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class update_external : public fb::protocol::header
{
public:
#ifndef BOT
    const fb::game::character& ch;
    const fb::game::object&    to;
    const bool                 light;
#else
    bool        is_light;
    uint16_t    x;
    uint16_t    y;
    uint8_t     direction;
    uint32_t    sequence;
    uint8_t     disguised;
    uint8_t     sex;
    uint8_t     state;
    uint16_t    look;
    uint8_t     color;
    uint8_t     armor_dress;
    uint8_t     armor_color;
    uint16_t    weapon_dress;
    uint8_t     weapon_color;
    uint8_t     shield_dress;
    uint8_t     shield_color;
    uint8_t     head_marker;
    std::string name;
#endif

public:
#ifndef BOT
    update_external(const fb::game::character& ch, const fb::game::object& to, bool light = false);
    update_external(const update_external&) = delete;
#else
    update_external() = default;
#endif

private:
#ifndef BOT
    HEAD_MARKER head_marker() const;
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