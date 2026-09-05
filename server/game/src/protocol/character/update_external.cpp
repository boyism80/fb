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

template <CLIENT_VERSION V>
update_external<V>::update_external(const fb::game::character& ch, const fb::game::object& to) :
    oid(ch.oid()),
    head_marker(compute_head_marker(ch, to)),
    name(ch.name()),
    appearance(),
    ui_mode(to.is(OBJECT_TYPE::CHARACTER) ? static_cast<const fb::game::character&>(to).ui_mode : CLIENT_UI_MODE::OLD)
{
    if (ch.mimicry().has_value())
    {
        this->appearance = ch.mimicry().value();
        if (this->appearance.state.has_value() == false)
            this->appearance.state = ch.state();
    }
    else
    {
        this->appearance = fb::game::character_appearance<V>::from(ch);
    }

    this->appearance.speed = ch.stat.speed();
    this->appearance.state = ch.state_to(to, this->appearance.state.value_or(ch.state()));
    if constexpr (V == CLIENT_VERSION::v651)
        this->appearance.face = ch.face();
}

template <CLIENT_VERSION V>
update_external<V>::update_external(uint32_t                                 oid,
                                    HEAD_MARKER                              head_marker,
                                    std::string                              name,
                                    const fb::game::character_appearance<V>& appearance,
                                    CLIENT_UI_MODE                           ui_mode) :
    oid(oid),
    head_marker(head_marker),
    name(std::move(name)),
    appearance(appearance),
    ui_mode(ui_mode)
{ }

template <CLIENT_VERSION V>
void update_external<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
    writer.write<bool>(this->appearance.disguise.has_value());
    writer.write<uint8_t>(static_cast<uint8_t>(this->appearance.gender));
    writer.write<uint8_t>(static_cast<uint8_t>(this->appearance.state.value_or(STATE::NORMAL)));
    if (this->appearance.disguise.has_value())
    {
        writer.write<uint16_t>(this->appearance.disguise.value());
        writer.write<uint8_t>(this->appearance.hair_color.value_or(this->appearance.armor_color.value_or(0x00)));
    }
    else
    {
        writer.write<uint16_t>(this->appearance.hair);
        writer.write<uint8_t>(this->appearance.hair_color.value_or(0x00));

        if (this->appearance.armor.has_value())
        {
            writer.write<uint8_t>(this->appearance.armor.value());
            writer.write<uint8_t>(this->appearance.armor_color.value_or(0x00));
        }
        else
        {
            writer.write<uint8_t>(static_cast<uint8_t>(this->appearance.gender));
            writer.write<uint8_t>(0x00);
        }

        if (this->appearance.weapon.has_value())
        {
            writer.write<uint16_t>(this->appearance.weapon.value());
            writer.write<uint8_t>(this->appearance.weapon_color.value_or(0x00));
        }
        else
        {
            writer.write<uint16_t>(0xFFFF);
            writer.write<uint8_t>(0x00);
        }

        if (this->appearance.shield.has_value())
        {
            writer.write<uint8_t>(this->appearance.shield.value());
            writer.write<uint8_t>(this->appearance.shield_color.value_or(0x00));
        }
        else
        {
            writer.write<uint8_t>(0xFF);
            writer.write<uint8_t>(0x00);
        }
    }

    writer.write<uint8_t>(static_cast<uint8_t>(this->head_marker));
    writer.write<std::string, uint8_t>(this->name);
}

template <>
void update_external<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
    writer.write<bool>(this->appearance.disguise.has_value());
    this->appearance.serialize(writer, this->ui_mode);
}

template class update_external<CLIENT_VERSION::v550>;
template class update_external<CLIENT_VERSION::v565>;
template class update_external<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
#else

namespace fb::protocol::game::response {

template <CLIENT_VERSION V>
void update_external<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid       = reader.read<uint32_t>();
    this->disguised = reader.read<uint8_t>();
    this->gender    = static_cast<GENDER>(reader.read<uint8_t>());
    this->state     = static_cast<STATE>(reader.read<uint8_t>());

    if (this->disguised)
    {
        this->look  = reader.read<uint16_t>();
        this->color = reader.read<uint8_t>();
    }
    else
    {
        this->look         = reader.read<uint16_t>();
        this->color        = reader.read<uint8_t>();
        this->armor_dress  = reader.read<uint8_t>();
        this->armor_color  = reader.read<uint8_t>();
        this->weapon_dress = reader.read<uint16_t>();
        this->weapon_color = reader.read<uint8_t>();
        this->shield_dress = reader.read<uint8_t>();
        this->shield_color = reader.read<uint8_t>();
    }

    this->head_marker = reader.read<uint8_t>();
    this->name        = reader.read<std::string, uint8_t>();
}

template <>
void update_external<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid       = reader.read<uint32_t>();
    this->disguised = reader.read<uint8_t>();
    this->gender    = static_cast<GENDER>(reader.read<uint8_t>());
    this->state     = static_cast<STATE>(reader.read<uint8_t>());

    if (this->disguised)
    {
        this->look  = reader.read<uint16_t>();
        this->color = reader.read<uint8_t>();
    }
    else
    {
        reader.read<uint16_t>();
        reader.read<uint8_t>();
        this->look  = reader.read<uint16_t>();
        this->color = reader.read<uint8_t>();
        reader.read<uint8_t>();
        this->armor_dress  = reader.read<uint8_t>();
        this->armor_color  = reader.read<uint8_t>();
        this->weapon_dress = reader.read<uint16_t>();
        this->weapon_color = reader.read<uint8_t>();
        auto shield        = reader.read<uint16_t>();
        this->shield_dress = static_cast<uint8_t>(shield);
        this->shield_color = reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint16_t>();
        reader.read<uint8_t>();
    }
}

template class update_external<CLIENT_VERSION::v550>;
template class update_external<CLIENT_VERSION::v565>;
template class update_external<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
#endif
