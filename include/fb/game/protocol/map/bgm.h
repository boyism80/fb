#ifndef __PROTOCOL_GAME_BGM_H__
#define __PROTOCOL_GAME_BGM_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/game/protocol/audio.h>
#ifndef BOT
#include <fb/game/map.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class map_bgm : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode         = 0x19;
    static constexpr uint8_t DEFAULT_VOLUME = 100;
    static constexpr uint8_t TLV_SIZE_BGM   = 0x05; // bgm + bgm_dup + volume

public:
#ifndef BOT
    const uint16_t bgm;
    const uint8_t  volume;
#else
    uint16_t bgm;
    uint8_t  volume = DEFAULT_VOLUME;
#endif

public:
#ifndef BOT
    map_bgm(uint16_t bgm, uint8_t volume = DEFAULT_VOLUME);
#else
    map_bgm() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

// Opcode 0x19 / AUDIO::BGM_STOP — body differs from play (no volume TLV).
// bgm_id == 0 → client stops all BGM; non-zero → stop/control that id.
class map_bgm_stop : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x19;
    // Client reads size then ignores it for stop; 2 = following u16.
    static constexpr uint8_t TLV_SIZE_BGM_STOP = 0x02;

public:
#ifndef BOT
    const uint16_t bgm_id;
#else
    uint16_t bgm_id = 0;
#endif

public:
#ifndef BOT
    explicit map_bgm_stop(uint16_t bgm_id = 0);
#else
    map_bgm_stop() = default;
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
