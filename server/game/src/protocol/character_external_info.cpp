#include <fb/game/protocol/character/external_info.h>
#ifndef BOT
#include <fb/game/server.h>
#endif

using table = fb::model::table;
using namespace fb::protocol::game::response;

#ifndef BOT
external_info::external_info(const fb::game::character& ch, const fb::game::object& to) :
    ch(ch),
    to(to)
{ }
#endif

#ifndef BOT
async::task<void> external_info::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string>(this->ch.title());

    auto& clan_id = this->ch.clan_id();
    if (clan_id.has_value())
    {
        this->ch.server.clans.read(clan_id.value(), [&writer](auto& clan) {
            writer.write<std::string>(clan->name());
            writer.write<std::string>(clan->title().value_or(""));
        });
    }
    else
    {
        writer.write<std::string>("");
        writer.write<std::string>("");
    }

    // Class name
    const auto& class_name = table::promotion[this->ch.cls()][this->ch.promotion()].name;
    writer.write<std::string>(class_name);      // Job/class
    writer.write<std::string>(this->ch.name()); // Character name

    auto appearance   = std::static_pointer_cast<fb::game::character_appearance>(this->ch.appearance());
    appearance->state = this->ch.state_to(this->to, appearance->state.value_or(this->ch.state()));
    auto disguised    = appearance->disguise.has_value();
    writer.write<uint8_t>(disguised);
    writer.write<uint8_t>(static_cast<uint8_t>(appearance->gender));
    writer.write<uint8_t>(static_cast<uint8_t>(appearance->state.value_or(this->ch.state())));

    auto armor  = this->ch.items.armor();  // Armor
    auto weapon = this->ch.items.weapon(); // Weapon
    auto shield = this->ch.items.shield(); // Shield
    if (disguised)
    {
        writer.write<uint16_t>(appearance->disguise.value());
        writer.write<uint8_t>(appearance->hair_color.value_or(appearance->armor_color.value_or(0x00)));
    }
    else
    {
        writer.write<uint16_t>(appearance->hair);
        writer.write<uint8_t>(appearance->hair_color.value_or(0x00));

        writer.write<uint8_t>(appearance->armor.value_or(static_cast<uint8_t>(appearance->gender)));
        writer.write<uint8_t>(appearance->armor_color.value_or(0x00));

        writer.write<uint16_t>(appearance->weapon.value_or(0xFFFF));
        writer.write<uint8_t>(appearance->weapon_color.value_or(0x00));

        writer.write<uint8_t>(appearance->shield.value_or(0xFF));
        writer.write<uint8_t>(appearance->shield_color.value_or(0x00));
    }

    // Equipment info
    auto sstream = std::stringstream();
    auto helmet  = this->ch.items.helmet(); // Helmet
    writer.write<uint16_t>(helmet != nullptr ? helmet->look() : 0xFFFF);
    writer.write<uint8_t>(helmet != nullptr ? helmet->color() : 0x00);

    auto ring_l = this->ch.items.ring(EQUIPMENT_POSITION::LEFT); // Left ring
    writer.write<uint16_t>(ring_l != nullptr ? ring_l->look() : 0xFFFF);
    writer.write<uint8_t>(ring_l != nullptr ? ring_l->color() : 0x00);

    auto ring_r = this->ch.items.ring(EQUIPMENT_POSITION::RIGHT); // Right ring
    writer.write<uint16_t>(ring_r != nullptr ? ring_r->look() : 0xFFFF);
    writer.write<uint8_t>(ring_r != nullptr ? ring_r->color() : 0x00);

    auto aux_l = this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT); // Auxiliary left
    writer.write<uint16_t>(aux_l != nullptr ? aux_l->look() : 0xFFFF);
    writer.write<uint8_t>(aux_l != nullptr ? aux_l->color() : 0x00);

    auto aux_r = this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT); // Auxiliary right
    writer.write<uint16_t>(aux_r != nullptr ? aux_r->look() : 0xFFFF);
    writer.write<uint8_t>(aux_r != nullptr ? aux_r->color() : 0x00);

    // Equipment info text (client display)
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
    writer.write<uint32_t>(0x00000000); // unknown

    // Achievements
    writer.write<uint8_t>((uint8_t)this->ch.achievements.size());
    for (auto& [_, achievement] : this->ch.achievements)
    {
        writer.write<uint8_t>(achievement->icon);
        writer.write<uint8_t>(achievement->color);
        writer.write<std::string>(achievement->text);
    }
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> external_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
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

    // Read equipment information
    if (this->disguised)
    {
        // In disguise state, only read armor_color
        this->armor_color = reader.read<uint8_t>();
    }
    else
    {
        // In normal state, read all equipment information
        this->armor_dress = reader.read<uint8_t>();
        this->armor_color = reader.read<uint8_t>();

        this->weapon_dress = reader.read<uint16_t>();
        this->weapon_color = reader.read<uint8_t>();

        this->shield_dress = reader.read<uint8_t>();
        this->shield_color = reader.read<uint8_t>();
    }

    // Additional equipment information (helmet, rings, auxiliary equipment)
    this->helmet.look  = reader.read<uint16_t>();
    this->helmet.color = reader.read<uint8_t>();

    this->ring_left.look  = reader.read<uint16_t>();
    this->ring_left.color = reader.read<uint8_t>();

    this->ring_right.look  = reader.read<uint16_t>();
    this->ring_right.color = reader.read<uint8_t>();

    this->aux_left.look  = reader.read<uint16_t>();
    this->aux_left.color = reader.read<uint8_t>();

    this->aux_right.look  = reader.read<uint16_t>();
    this->aux_right.color = reader.read<uint8_t>();

    // Equipment information text
    this->equipment_text = reader.read<std::string, uint8_t>();

    // Additional information
    this->oid          = reader.read<uint32_t>();
    this->group_option = reader.read<uint8_t>();
    this->trade_option = reader.read<uint8_t>();
    this->unknown      = reader.read<uint32_t>();

    // Achievement information
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
    reader.read<uint8_t>(); // Final 0x00
}
#endif
