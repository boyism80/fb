#ifndef __PROTOCOL_GAME_SOUND_H__
#define __PROTOCOL_GAME_SOUND_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class sound : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x19;

public:
#ifndef BOT
    const fb::game::object& me;
    const SOUND             value;
#else
    uint32_t sequence;
    SOUND    value;
#endif

public:
#ifndef BOT
    sound(const fb::game::object& me, SOUND value);
#else
    sound() = default;
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