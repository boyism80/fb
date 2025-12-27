#ifndef __PROTOCOL_GAME_POSITION_H__
#define __PROTOCOL_GAME_POSITION_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#include <fb/game/map.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class position : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x04;

public:
#ifndef BOT
    const fb::game::character& ch;
#else
    fb::model::point<uint16_t> abs;
    fb::model::point<uint16_t> rel;
#endif

public:
#ifndef BOT
    position(const fb::game::character& ch);
#else
    position() = default;
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