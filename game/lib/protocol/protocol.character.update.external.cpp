#include <fb/game/protocol/character/update_external.h>
#ifndef BOT
#include <fb/game/group.h>
#endif

namespace fb::protocol::game::response {

#ifndef BOT
update_external::update_external(const fb::game::character& ch, const fb::game::object& to, bool light) :
    ch(ch),
    to(to),
    light(light)
{ }
#endif

#ifndef BOT
HEAD_MARKER update_external::head_marker() const
{
    if (&this->ch == &this->to)
        return HEAD_MARKER::NONE;

    if (this->to.is(OBJECT_TYPE::CHARACTER) == false)
        return HEAD_MARKER::NONE;

    auto& you = static_cast<const fb::game::character&>(this->to);
    if (this->ch.clan() != nullptr && this->ch.clan() == you.clan())
        return HEAD_MARKER::BLUE;

    return HEAD_MARKER::NONE;
}
#endif

#ifndef BOT
async::task<void> update_external::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    auto map = this->ch.map();
    if (map == nullptr)
        co_return;

    if (this->light == false)
    {
        writer.write<uint8_t>(0x33);                                       // id
        writer.write<uint16_t>(this->ch.x());                              // x
        writer.write<uint16_t>(this->ch.y());                              // y
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.direction())); // side
    }
    else
    {
        writer.write<uint8_t>(0x1D); // id
    }

    writer.write<uint32_t>(this->ch.sequence());
    writer.write<uint8_t>(this->ch.state() == STATE::DISGUISE);  // 변신유무
    writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex())); // sex
    writer.write<uint8_t>(static_cast<uint8_t>(this->ch.state_to(this->to)));
    if (this->ch.state() == STATE::DISGUISE)
    {
        writer.write<uint16_t>(this->ch.disguise().value());
        writer.write<uint8_t>(this->ch.current_armor_color());
    }
    else
    {
        writer.write<uint16_t>(this->ch.look()); // face
        writer.write<uint8_t>(this->ch.color()); // hair color

        auto armor = this->ch.items.armor();
        if (armor != nullptr)
        {
            writer.write<uint8_t>((uint8_t)armor->based<fb::model::armor>().dress);
            writer.write<uint8_t>(ch.current_armor_color());
        }
        else
        {
            writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex()));
            writer.write<uint8_t>(0x00);
        }

        auto weapon = this->ch.items.weapon();
        if (weapon != nullptr)
        {
            writer.write<uint16_t>(weapon->based<fb::model::weapon>().dress);
            writer.write<uint8_t>(weapon->color());
        }
        else
        {
            writer.write<uint16_t>(0xFFFF);
            writer.write<uint8_t>(0x00);
        }

        auto shield = this->ch.items.shield();
        if (shield != nullptr)
        {
            writer.write<uint8_t>((uint8_t)shield->based<fb::model::shield>().dress);
            writer.write<uint8_t>(shield->color());
        }
        else
        {
            writer.write<uint8_t>(0xFF); // about shield
            writer.write<uint8_t>(0x00);
        }
    }

    writer.write<uint8_t>(static_cast<uint8_t>(this->head_marker())); // head mark
    writer.write<std::string, uint8_t>(this->ch.name());              // name
}
#else
async::task<void> update_external::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);

    uint8_t id     = reader.read<uint8_t>();
    this->is_light = (id == 0x1D);

    if (!this->is_light)
    {
        this->x         = reader.read<uint16_t>();
        this->y         = reader.read<uint16_t>();
        this->direction = reader.read<uint8_t>();
    }

    this->sequence  = reader.read<uint32_t>();
    this->disguised = reader.read<uint8_t>();
    this->sex       = reader.read<uint8_t>();
    this->state     = reader.read<uint8_t>();

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
#endif
}; // namespace fb::protocol::game::response