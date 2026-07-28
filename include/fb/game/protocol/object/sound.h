#ifndef __PROTOCOL_GAME_SOUND_H__
#define __PROTOCOL_GAME_SOUND_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/game/protocol/audio.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class sound : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode         = 0x19;
    static constexpr uint8_t DEFAULT_VOLUME = 100;
    // Bytes after subtype for sound_u16 + volume_u8 (client size field).
    static constexpr uint8_t TLV_SIZE_SOUND = 0x03;

public:
#ifndef BOT
    const fb::game::object& me;
    const SOUND             value;
    const uint8_t           volume;
#else
    uint32_t oid;
    SOUND    value;
    uint8_t  volume = DEFAULT_VOLUME;
#endif

public:
#ifndef BOT
    sound(const fb::game::object& me, SOUND value, uint8_t volume = DEFAULT_VOLUME);
#else
    sound() = default;
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
