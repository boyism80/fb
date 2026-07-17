#include <fb/game/protocol/character/internal_info.h>
#include <fb/game/client_amount.h>
#ifndef BOT
#include <fb/game/group.h>
#include <fb/game/server.h>
#endif

using table = fb::model::table;
using namespace fb::protocol::game::response;

#ifndef BOT
void internal_info::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<int8_t>(this->ch.stat.phydef());
    writer.write<int8_t>(this->ch.stat.dam());
    writer.write<int8_t>(this->ch.stat.hit());

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
    writer.write<std::string>(this->ch.title());

    auto  sstream  = std::stringstream();
    auto& marriage = this->ch.marriage();
    if (marriage.spouse_id.has_value())
    {
        sstream << "배우자: " << marriage.spouse_name << std::endl;
    }

    auto& group_id = this->ch.group_id();
    if (group_id.has_value())
    {
        auto  guard = this->ch.server.groups.enter_read(group_id.value());
        auto& group = guard.value();
        sstream << _TEXT(MESSAGE_GROUP_MEMBERS_HEADER) << std::endl << "  * " << group->master() << std::endl;

        auto master_name = group->master();
        for (auto& member : group->members())
        {
            if (member != master_name)
                sstream << "    " << member << std::endl;
        }
    }
    else
    {
        sstream << _TEXT(MESSAGE_GROUP_NONE);
    }
    writer.write<std::string>(sstream.str());
    writer.write<uint8_t>(this->ch.option(OPTION::GROUP));

    writer.write<uint32_t>(fb::game::encode_client_amount(this->ch.experience_remained()));

    auto& class_name = table::promotion[this->ch.cls()][this->ch.promotion()].name;
    writer.write<std::string>(class_name);

    auto equipments =
        std::array<std::shared_ptr<fb::game::equipment>, 5>{this->ch.items.helmet(),
                                                            this->ch.items.ring(EQUIPMENT_POSITION::LEFT),
                                                            this->ch.items.ring(EQUIPMENT_POSITION::RIGHT),
                                                            this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT),
                                                            this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT)};
    for (int i = 0, size = equipments.size(); i < size; i++)
    {
        if (equipments[i] == nullptr)
        {
            writer.write<uint16_t>(0xFFFF);
            writer.write<uint8_t>(0x00);
        }
        else
        {
            writer.write<uint16_t>(equipments[i]->look());
            writer.write<uint8_t>(equipments[i]->color());
        }
    }

    writer.write<uint8_t>(0x00); // fixed
    writer.write<uint8_t>(this->ch.option(OPTION::TRADE));
    writer.write<uint8_t>(this->ch.option(OPTION::PK_PROTECT));

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
void internal_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->phydef       = reader.read<int8_t>();
    this->dam          = reader.read<int8_t>();
    this->hit          = reader.read<int8_t>();
    this->clan_name    = reader.read<std::string, uint8_t>();
    this->clan_title   = reader.read<std::string, uint8_t>();
    this->title        = reader.read<std::string, uint8_t>();
    this->group_info   = reader.read<std::string, uint8_t>();
    this->group_option = reader.read<uint8_t>();
    this->remained_exp = reader.read<uint32_t>();
    this->class_name   = reader.read<std::string, uint8_t>();

    // Equipment information (5 slots)
    this->equipments.clear();
    for (int i = 0; i < 5; i++)
    {
        equipment_data equip;
        equip.look  = reader.read<uint16_t>();
        equip.color = reader.read<uint8_t>();
        this->equipments.push_back(equip);
    }

    reader.read<uint8_t>(); // fixed 0x00
    this->trade_option      = reader.read<uint8_t>();
    this->pk_protect_option = reader.read<uint8_t>();

    // Fully implement achievement information
    uint8_t achievement_count = reader.read<uint8_t>();
    this->achievements.clear();
    for (int i = 0; i < achievement_count; i++)
    {
        achievement_data achievement;
        achievement.look  = reader.read<uint8_t>();
        achievement.color = reader.read<uint8_t>();
        achievement.text  = reader.read<std::string, uint8_t>();
        this->achievements.push_back(achievement);
    }
    reader.read<uint8_t>(); // Final 0x00
}
#endif