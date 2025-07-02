#ifndef __PROTOCOL_GAME_SHOW_HP_H__
#define __PROTOCOL_GAME_SHOW_HP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/life.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class update_hp : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x13;

public:
#ifndef BOT
    const fb::game::life& me;
    const uint32_t        damage;
    const bool            critical;
#else
    uint32_t oid;
    uint32_t damage;
    uint8_t  critical;
    uint8_t  percent;
#endif

public:
#ifndef BOT
    update_hp(const fb::game::life& me, uint32_t damage, bool critical);
#else
    update_hp() = default;
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