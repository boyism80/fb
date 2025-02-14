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
    bool        is_detected() const;
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