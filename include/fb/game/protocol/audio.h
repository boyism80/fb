#ifndef __PROTOCOL_GAME_AUDIO_H__
#define __PROTOCOL_GAME_AUDIO_H__

#include <cstdint>

namespace fb::protocol::game::response {

// Shared opcode 0x19 payload[0] multiplex.
// SOUND / BGM / BGM_STOP use different body layouts after subtype (+ size).
enum class AUDIO : uint8_t
{
    SOUND    = 0x00, // SFX: size + sound_u16 + volume_u8 + target/effect TLVs
    BGM      = 0x01, // BGM play: size(5) + bgm_u16 + bgm_u16 + volume_u8
    BGM_STOP = 0x02, // BGM stop: size unused by client; then bgm_u16 (0 = stop all)
};

} // namespace fb::protocol::game::response

#endif
