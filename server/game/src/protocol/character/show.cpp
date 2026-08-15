#include <fb/game/protocol/character/show.h>
#ifndef BOT
#include <fb/game/server.h>

namespace fb::protocol::game::response {

template <CLIENT_VERSION V>
show<V>::show(const fb::game::character& ch, const fb::game::object& to) :
    oid(ch.oid()),
    position(ch.position()),
    direction(ch.direction()),
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
        this->appearance.gender      = ch.gender();
        this->appearance.state       = ch.state();
        this->appearance.hair        = ch.look();
        this->appearance.hair_color  = ch.color();
        this->appearance.armor_color = ch.armor_color();
        this->appearance.disguise    = std::nullopt;

        if (ch.items.armor() != nullptr)
        {
            this->appearance.armor = static_cast<uint8_t>(ch.items.armor()->model().dress);
            if (this->appearance.armor_color.has_value() == false)
                this->appearance.armor_color = ch.items.armor()->model().color;
        }

        if (ch.items.weapon() != nullptr)
        {
            this->appearance.weapon = ch.items.weapon()->model().dress;
            this->appearance.weapon_color =
                ch.weapon_color().value_or(static_cast<uint8_t>(ch.items.weapon()->color()));
        }

        if (ch.items.shield() != nullptr)
        {
            this->appearance.shield       = ch.items.shield()->model().dress;
            this->appearance.shield_color = ch.shield_color().value_or(ch.items.shield()->color());
        }
    }

    this->appearance.speed = ch.stat.speed();
    this->appearance.state = ch.state_to(to, this->appearance.state.value_or(ch.state()));
}

template <CLIENT_VERSION V>
show<V>::show(uint32_t                                 oid,
              const fb::model::point<uint16_t>&        position,
              DIRECTION                                direction,
              HEAD_MARKER                              head_marker,
              std::string                              name,
              const fb::game::character_appearance<V>& appearance,
              CLIENT_UI_MODE                           ui_mode) :
    oid(oid),
    position(position),
    direction(direction),
    head_marker(head_marker),
    name(std::move(name)),
    appearance(appearance),
    ui_mode(ui_mode)
{ }

template <CLIENT_VERSION V>
void show<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
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
void show<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
    writer.write<uint32_t>(this->oid);
    writer.write<bool>(this->appearance.disguise.has_value());
    this->appearance.serialize(writer, this->ui_mode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->head_marker));
    writer.write<std::string, uint8_t>(this->name);
}

template class show<CLIENT_VERSION::v550>;
template class show<CLIENT_VERSION::v565>;
template class show<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
#else

namespace fb::protocol::game::response {

template <CLIENT_VERSION V>
void show<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->x         = reader.read<uint16_t>();
    this->y         = reader.read<uint16_t>();
    this->direction = reader.read<uint8_t>();
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
void show<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->x         = reader.read<uint16_t>();
    this->y         = reader.read<uint16_t>();
    this->direction = reader.read<uint8_t>();
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

    this->head_marker = reader.read<uint8_t>();
    this->name        = reader.read<std::string, uint8_t>();
}

template class show<CLIENT_VERSION::v550>;
template class show<CLIENT_VERSION::v565>;
template class show<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
#endif
