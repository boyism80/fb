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

namespace {

void write_appearance_v651(fb::stream_writer<big_endian>& writer, const character_appearance& app)
{
    writer.write<uint8_t>(static_cast<uint8_t>(app.gender));
    writer.write<uint8_t>(static_cast<uint8_t>(app.state.value_or(STATE::NORMAL)));

    if (app.disguise.has_value())
    {
        // type1 — 5B total: gender + state + look u16 + color
        writer.write<uint16_t>(app.disguise.value());
        writer.write<uint8_t>(app.hair_color.value_or(app.armor_color.value_or(0x00)));
        return;
    }

    // type0 — 23B
    writer.write<uint16_t>(app.unknown_ridable_id);
    writer.write<uint8_t>(app.unknown_anim_base);
    writer.write<uint16_t>(app.hair);
    writer.write<uint8_t>(app.unknown_hair_style);
    writer.write<uint8_t>(app.unknown_face_hair_tint);
    writer.write<uint8_t>(
        app.armor_color.value_or(app.armor.has_value() ? app.armor.value() : static_cast<uint8_t>(app.gender)));
    writer.write<uint8_t>(app.unknown_body_color);
    writer.write<uint16_t>(app.weapon.value_or(0xFFFF));
    writer.write<uint8_t>(app.weapon_color.value_or(0x00));
    writer.write<uint16_t>(app.shield.has_value() ? static_cast<uint16_t>(app.shield.value()) : 0xFFFF);
    writer.write<uint8_t>(app.shield_color.value_or(0x00));
    writer.write<uint8_t>(app.unknown_hair_to_hat);
    writer.write<uint8_t>(app.unknown_helmet);
    writer.write<uint8_t>(app.unknown_helmet_color);
    writer.write<uint16_t>(app.unknown_accessory_pack);
    writer.write<uint8_t>(app.unknown_accessory_color);
}

} // namespace

template <>
void appearance_serializer<true, CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->appearance.disguise.has_value() ? 1 : 0);
    write_appearance_v651(writer, this->appearance);
    writer.write<uint8_t>(static_cast<uint8_t>(this->head_marker));
    writer.write<std::string, uint8_t>(this->name);
}

template <>
void appearance_serializer<false, CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->appearance.disguise.has_value() ? 1 : 0);
    write_appearance_v651(writer, this->appearance);
    // 0x1D: trailing marker+name NOT WRITTEN
}

} // namespace fb::protocol::game::response
#endif
