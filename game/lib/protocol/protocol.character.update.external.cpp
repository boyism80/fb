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
bool update_external::clock_visible() const
{
    if (&this->ch == &this->to)
        return true;

    if (this->to.is(OBJECT_TYPE::CHARACTER) == false)
        return false;

    // TODO: to 에게 걸린 버프가 있어서 그게 투명 다 감지하는 버프면
    // return true

    auto& mine = this->ch.group();
    if (mine == nullptr)
        return false;

    auto& your = static_cast<const fb::game::character&>(this->to).group();
    if (your == nullptr)
        return false;

    return mine.get() == your.get();
}

HEAD_MARKER update_external::head_marker() const
{
    if (&this->ch == &this->to)
        return HEAD_MARKER::NONE;

    if (this->to.is(OBJECT_TYPE::CHARACTER) == false)
        return HEAD_MARKER::NONE;

    auto& you = static_cast<const fb::game::character&>(this->to);
    if (this->ch.clan() == you.clan())
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

    switch (this->ch.state())
    {
    case STATE::HALF_CLOACK:
    {
        if (this->clock_visible())
            writer.write<uint8_t>(static_cast<uint8_t>(STATE::HALF_CLOACK));
        else
            writer.write<uint8_t>(static_cast<uint8_t>(STATE::CLOACK));
    }
    break;

    default:
    {
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.state()));
    }
    break;
    }

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
            writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex())); // sex
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
    // TODO: deserialize bytes
}
#endif
}; // namespace fb::protocol::game::response