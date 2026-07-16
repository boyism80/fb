#ifndef __PROTOCOL_GAME_BGM_H__
#define __PROTOCOL_GAME_BGM_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/map.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class map_bgm : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x19;

public:
#ifndef BOT
    const uint16_t bgm;
    const uint8_t  volume;
#else
    uint16_t bgm;
    uint8_t  volume;
#endif

public:
#ifndef BOT
    map_bgm(uint16_t bgm, uint8_t volume = 100);
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

} // namespace fb::protocol::game::response

#endif