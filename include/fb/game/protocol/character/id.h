#ifndef __PROTOCOL_GAME_ID_H__
#define __PROTOCOL_GAME_ID_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class id : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x05;

public:
#ifndef BOT
    const fb::game::character& ch;
#else
    uint32_t sequence  = 0;
    uint32_t direction = 0;
    uint8_t  cls       = 0;
#endif

public:
#ifndef BOT
    id(const fb::game::character& ch);
#else
    id() = default;
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