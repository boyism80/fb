#ifndef __PROTOCOL_GAME_DIE_H__
#define __PROTOCOL_GAME_DIE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/life.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class die : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x5F;

public:
#ifndef BOT
    const uint32_t id;
#else

#endif

public:
#ifndef BOT
    die(const fb::game::life& life);
    die(uint32_t id);
#else
    die() = default;
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