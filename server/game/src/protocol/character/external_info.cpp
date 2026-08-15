#include <fb/game/protocol/character/external_info.h>
#ifndef BOT
#include <fb/game/server.h>
#include <sstream>
#endif

using table = fb::model::table;

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
external_info<V>::external_info(const fb::game::character& ch, const fb::game::object& to) :
    ch(ch),
    to(to)
{ }

template <CLIENT_VERSION V>
void external_info<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string>(this->ch.title());

    auto& clan_id = this->ch.clan_id();
    if (clan_id.has_value())
    {
        auto guard = this->ch.server.clans.enter_read(clan_id.value());
        writer.write<std::string>(guard.value()->name());
        writer.write<std::string>(guard.value()->title().value_or(""));
    }
    else
    {
        writer.write<std::string>("");
        writer.write<std::string>("");
    }

    writer.write<std::string>(table::promotion[this->ch.cls()][this->ch.promotion()].name);
    writer.write<std::string>(this->ch.name());

    auto src         = std::static_pointer_cast<fb::game::character_appearance<>>(this->ch.appearance());
    auto appearance  = *src;
    appearance.state = this->ch.state_to(this->to, appearance.state.value_or(this->ch.state()));
    writer.write<bool>(appearance.disguise.has_value());
    writer.write<uint8_t>(static_cast<uint8_t>(appearance.gender));
    writer.write<uint8_t>(static_cast<uint8_t>(appearance.state.value_or(this->ch.state())));

    auto armor  = this->ch.items.armor();
    auto weapon = this->ch.items.weapon();
    auto shield = this->ch.items.shield();
    auto helmet = this->ch.items.helmet();
    auto ring_l = this->ch.items.ring(EQUIPMENT_POSITION::LEFT);
    auto ring_r = this->ch.items.ring(EQUIPMENT_POSITION::RIGHT);
    auto aux_l  = this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT);
    auto aux_r  = this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT);

    if (appearance.disguise.has_value())
    {
        writer.write<uint16_t>(appearance.disguise.value());
        writer.write<uint8_t>(appearance.hair_color.value_or(appearance.armor_color.value_or(0x00)));
    }
    else
    {
        writer.write<uint16_t>(appearance.hair);
        writer.write<uint8_t>(appearance.hair_color.value_or(0x00));
        writer.write<uint8_t>(appearance.armor.value_or(static_cast<uint8_t>(appearance.gender)));
        writer.write<uint8_t>(appearance.armor_color.value_or(0x00));
        writer.write<uint16_t>(appearance.weapon.value_or(0xFFFF));
        writer.write<uint8_t>(appearance.weapon_color.value_or(0x00));
        writer.write<uint8_t>(appearance.shield.value_or(0xFF));
        writer.write<uint8_t>(appearance.shield_color.value_or(0x00));
    }

    writer.write<uint16_t>(helmet != nullptr ? helmet->look() : 0xFFFF);
    writer.write<uint8_t>(helmet != nullptr ? helmet->color() : 0x00);
    writer.write<uint16_t>(ring_l != nullptr ? ring_l->look() : 0xFFFF);
    writer.write<uint8_t>(ring_l != nullptr ? ring_l->color() : 0x00);
    writer.write<uint16_t>(ring_r != nullptr ? ring_r->look() : 0xFFFF);
    writer.write<uint8_t>(ring_r != nullptr ? ring_r->color() : 0x00);
    writer.write<uint16_t>(aux_l != nullptr ? aux_l->look() : 0xFFFF);
    writer.write<uint8_t>(aux_l != nullptr ? aux_l->color() : 0x00);
    writer.write<uint16_t>(aux_r != nullptr ? aux_r->look() : 0xFFFF);
    writer.write<uint8_t>(aux_r != nullptr ? aux_r->color() : 0x00);

    auto sstream = std::stringstream();
    sstream << " w:무기  :" << (weapon != nullptr ? weapon->name() : "없음") << std::endl;
    sstream << " a:갑옷  :" << (armor != nullptr ? armor->name() : "없음") << std::endl;
    sstream << " s:방패  :" << (shield != nullptr ? shield->name() : "없음") << std::endl;
    sstream << " h:머리  :" << (helmet != nullptr ? helmet->name() : "없음") << std::endl;
    sstream << " l:왼손  :" << (ring_l != nullptr ? ring_l->name() : "없음") << std::endl;
    sstream << " r:오른손:" << (ring_r != nullptr ? ring_r->name() : "없음") << std::endl;
    sstream << " [:보조1 :" << (aux_l != nullptr ? aux_l->name() : "없음") << std::endl;
    sstream << " ]:보조2 :" << (aux_r != nullptr ? aux_r->name() : "없음") << std::endl;
    writer.write<std::string>(sstream.str());

    writer.write<uint32_t>(this->ch.oid());
    writer.write<uint8_t>(this->ch.option(OPTION::GROUP));
    writer.write<uint8_t>(this->ch.option(OPTION::TRADE));
    writer.write<uint32_t>(0x00000000);

    writer.write<uint8_t>((uint8_t)this->ch.achievements.size());
    for (auto& [_, achievement] : this->ch.achievements)
    {
        writer.write<uint8_t>(achievement->icon);
        writer.write<uint8_t>(achievement->color);
        writer.write<std::string>(achievement->text);
    }
}

template <>
void external_info<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string>(this->ch.title());

    auto& clan_id = this->ch.clan_id();
    if (clan_id.has_value())
    {
        auto guard = this->ch.server.clans.enter_read(clan_id.value());
        writer.write<std::string>(guard.value()->name());
        writer.write<std::string>(guard.value()->title().value_or(""));
    }
    else
    {
        writer.write<std::string>("");
        writer.write<std::string>("");
    }

    writer.write<std::string>(table::promotion[this->ch.cls()][this->ch.promotion()].name);
    writer.write<std::string>(this->ch.name());

    auto src         = std::static_pointer_cast<fb::game::character_appearance<>>(this->ch.appearance());
    auto appearance  = fb::game::character_appearance<CLIENT_VERSION::v651>(*src);
    appearance.speed = this->ch.stat.speed();
    appearance.state = this->ch.state_to(this->to, appearance.state.value_or(this->ch.state()));
    writer.write<bool>(appearance.disguise.has_value());

    auto ui_mode = this->to.is(OBJECT_TYPE::CHARACTER) ? static_cast<const fb::game::character&>(this->to).ui_mode
                                                       : fb::protocol::CLIENT_UI_MODE::OLD;
    appearance.serialize(writer, ui_mode);

    auto                                 armor  = this->ch.items.armor();
    auto                                 weapon = this->ch.items.weapon();
    auto                                 shield = this->ch.items.shield();
    auto                                 helmet = this->ch.items.helmet();
    auto                                 ring_l = this->ch.items.ring(EQUIPMENT_POSITION::LEFT);
    auto                                 ring_r = this->ch.items.ring(EQUIPMENT_POSITION::RIGHT);
    auto                                 aux_l  = this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT);
    auto                                 aux_r  = this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT);
    std::shared_ptr<fb::game::equipment> slots[] =
        {weapon, armor, shield, helmet, ring_l, ring_r, aux_l, aux_r, nullptr};
    for (auto& eq : slots)
    {
        if (eq == nullptr)
        {
            writer.write<uint16_t>(0);
            writer.write<uint8_t>(0);
            writer.write<std::string>("");
            writer.write<std::string>("");
            writer.write<uint32_t>(0);
        }
        else
        {
            writer.write<uint16_t>(eq->look());
            writer.write<uint8_t>(eq->color());
            writer.write<std::string>(eq->name());
            writer.write<std::string>("");
            writer.write<uint32_t>(0);
        }
    }

    auto sstream = std::stringstream();
    sstream << " w:무기  :" << (weapon != nullptr ? weapon->name() : "없음") << std::endl;
    sstream << " a:갑옷  :" << (armor != nullptr ? armor->name() : "없음") << std::endl;
    sstream << " s:방패  :" << (shield != nullptr ? shield->name() : "없음") << std::endl;
    sstream << " h:머리  :" << (helmet != nullptr ? helmet->name() : "없음") << std::endl;
    sstream << " l:왼손  :" << (ring_l != nullptr ? ring_l->name() : "없음") << std::endl;
    sstream << " r:오른손:" << (ring_r != nullptr ? ring_r->name() : "없음") << std::endl;
    sstream << " [:보조1 :" << (aux_l != nullptr ? aux_l->name() : "없음") << std::endl;
    sstream << " ]:보조2 :" << (aux_r != nullptr ? aux_r->name() : "없음") << std::endl;
    writer.write<std::string>(sstream.str());

    writer.write<uint32_t>(this->ch.oid());
    writer.write<uint8_t>(this->ch.option(OPTION::GROUP));
    writer.write<uint8_t>(this->ch.option(OPTION::TRADE));
    writer.write<uint8_t>(0);                     // skipped with trade (IDA add edi,2)
    writer.write<int16_t>(this->ch.reputation()); // 인품, this+0xB64
    writer.write<uint16_t>(0);
    writer.write<std::string>("");

    writer.write<uint8_t>((uint8_t)this->ch.achievements.size());
    for (auto& [_, achievement] : this->ch.achievements)
    {
        writer.write<uint8_t>(achievement->icon);
        writer.write<uint8_t>(achievement->color);
        writer.write<std::string>(achievement->text);
    }
}

template external_info<CLIENT_VERSION::v550>::external_info(const fb::game::character&, const fb::game::object&);
template external_info<CLIENT_VERSION::v565>::external_info(const fb::game::character&, const fb::game::object&);
template external_info<CLIENT_VERSION::v651>::external_info(const fb::game::character&, const fb::game::object&);
template void external_info<CLIENT_VERSION::v550>::serialize(fb::stream_writer<big_endian>&) const;
template void external_info<CLIENT_VERSION::v565>::serialize(fb::stream_writer<big_endian>&) const;
#else
template <CLIENT_VERSION V>
void external_info<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->title      = reader.read<std::string, uint8_t>();
    this->clan_name  = reader.read<std::string, uint8_t>();
    this->clan_title = reader.read<std::string, uint8_t>();
    this->class_name = reader.read<std::string, uint8_t>();
    this->name       = reader.read<std::string, uint8_t>();

    this->disguised = reader.read<uint8_t>();
    this->gender    = reader.read<uint8_t>();
    this->state     = reader.read<uint8_t>();

    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();

    if (this->disguised)
    {
        this->armor_color = reader.read<uint8_t>();
    }
    else
    {
        this->armor_dress  = reader.read<uint8_t>();
        this->armor_color  = reader.read<uint8_t>();
        this->weapon_dress = reader.read<uint16_t>();
        this->weapon_color = reader.read<uint8_t>();
        this->shield_dress = reader.read<uint8_t>();
        this->shield_color = reader.read<uint8_t>();
    }

    this->helmet.look      = reader.read<uint16_t>();
    this->helmet.color     = reader.read<uint8_t>();
    this->ring_left.look   = reader.read<uint16_t>();
    this->ring_left.color  = reader.read<uint8_t>();
    this->ring_right.look  = reader.read<uint16_t>();
    this->ring_right.color = reader.read<uint8_t>();
    this->aux_left.look    = reader.read<uint16_t>();
    this->aux_left.color   = reader.read<uint8_t>();
    this->aux_right.look   = reader.read<uint16_t>();
    this->aux_right.color  = reader.read<uint8_t>();

    this->equipment_text = reader.read<std::string, uint8_t>();
    this->oid            = reader.read<uint32_t>();
    this->group_option   = reader.read<uint8_t>();
    this->trade_option   = reader.read<uint8_t>();
    reader.read<uint32_t>();

    uint8_t achievement_count = reader.read<uint8_t>();
    this->achievements.clear();
    for (int i = 0; i < achievement_count; i++)
    {
        achievement_data achievement;
        achievement.icon  = reader.read<uint8_t>();
        achievement.color = reader.read<uint8_t>();
        achievement.text  = reader.read<std::string, uint8_t>();
        this->achievements.push_back(achievement);
    }
}

template <>
void external_info<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->title      = reader.read<std::string, uint8_t>();
    this->clan_name  = reader.read<std::string, uint8_t>();
    this->clan_title = reader.read<std::string, uint8_t>();
    this->class_name = reader.read<std::string, uint8_t>();
    this->name       = reader.read<std::string, uint8_t>();

    this->disguised = reader.read<uint8_t>();
    this->gender    = reader.read<uint8_t>();
    this->state     = reader.read<uint8_t>();

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

    equipment_data* slots[] = {nullptr,
                               nullptr,
                               nullptr,
                               &this->helmet,
                               &this->ring_left,
                               &this->ring_right,
                               &this->aux_left,
                               &this->aux_right,
                               nullptr};
    for (auto* dst : slots)
    {
        auto look   = reader.read<uint16_t>();
        auto color  = reader.read<uint8_t>();
        auto name_a = reader.read<std::string, uint8_t>();
        auto name_b = reader.read<std::string, uint8_t>();
        auto u32    = reader.read<uint32_t>();
        if (dst != nullptr)
        {
            dst->look           = look;
            dst->color          = color;
            dst->name_a         = name_a;
            dst->unknown_name_b = name_b;
            dst->unknown_u32    = u32;
        }
    }

    this->equipment_text = reader.read<std::string, uint8_t>();
    this->oid            = reader.read<uint32_t>();
    this->group_option   = reader.read<uint8_t>();
    this->trade_option   = reader.read<uint8_t>();
    reader.read<uint8_t>();
    this->reputation = reader.read<int16_t>();
    reader.read<uint16_t>();
    reader.read<std::string, uint8_t>();

    uint8_t achievement_count = reader.read<uint8_t>();
    this->achievements.clear();
    for (int i = 0; i < achievement_count; i++)
    {
        achievement_data achievement;
        achievement.icon  = reader.read<uint8_t>();
        achievement.color = reader.read<uint8_t>();
        achievement.text  = reader.read<std::string, uint8_t>();
        this->achievements.push_back(achievement);
    }
}

template void external_info<CLIENT_VERSION::v550>::deserialize(fb::stream_reader<big_endian>&);
template void external_info<CLIENT_VERSION::v565>::deserialize(fb::stream_reader<big_endian>&);
#endif

} // namespace fb::protocol::game::response
