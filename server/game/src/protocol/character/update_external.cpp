#include <fb/game/protocol/character/update_external.h>
#ifndef BOT
#include <fb/game/server.h>

namespace fb::protocol::game::response {

HEAD_MARKER compute_head_marker(const fb::game::character& ch, const fb::game::object& to)
{
    if (&ch == &to)
        return HEAD_MARKER::NONE;

    if (to.is(OBJECT_TYPE::CHARACTER) == false)
        return HEAD_MARKER::NONE;

    auto clan_id_1 = ch.clan_id();
    if (clan_id_1.has_value() == false)
        return HEAD_MARKER::NONE;

    auto& you       = static_cast<const fb::game::character&>(to);
    auto  clan_id_2 = you.clan_id();
    if (clan_id_2.has_value() == false)
        return HEAD_MARKER::NONE;

    if (clan_id_1 == clan_id_2)
        return HEAD_MARKER::BLUE;

    auto guard = ch.server.clans.try_enter_read(clan_id_1.value());
    if (guard.has_value() == false || guard->value() == nullptr)
        return HEAD_MARKER::NONE;

    auto& clan = guard->value();
    if (clan->is_hostile(clan_id_2.value()))
        return HEAD_MARKER::RED;

    if (clan->is_allied(clan_id_2.value()))
        return HEAD_MARKER::YELLOW;

    return HEAD_MARKER::NONE;
}

} // namespace fb::protocol::game::response
#endif
