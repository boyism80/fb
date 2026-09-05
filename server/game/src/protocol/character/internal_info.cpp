#include <fb/game/protocol/character/internal_info.h>
#include <fb/game/client_amount.h>
#ifndef BOT
#include <fb/game/group.h>
#include <fb/game/server.h>
#endif

using table = fb::model::table;

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
internal_info<V>::internal_info(fb::game::character& ch) :
    ch(ch)
{ }
#endif

#ifndef BOT
namespace {

template <fb::protocol::CLIENT_VERSION V, typename Writer>
void write_internal_info_header(Writer& writer, fb::game::character& ch)
{
    writer.template write<int8_t>(ch.stat.phydef());
    writer.template write<int8_t>(ch.stat.dam());
    writer.template write<int8_t>(ch.stat.hit());

    auto& clan_id = ch.clan_id();
    if (clan_id.has_value())
    {
        auto guard = ch.server.clans.enter_read(clan_id.value());
        writer.template write<std::string>(guard.value()->name());
        writer.template write<std::string>(guard.value()->title().value_or(""));
    }
    else
    {
        writer.template write<std::string>("");
        writer.template write<std::string>("");
    }
    writer.template write<std::string>(ch.title());

    auto& marriage = ch.marriage();
    if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
    {
        if (marriage.spouse_id.has_value())
            writer.template write<std::string>(std::string("배우자: ") + marriage.spouse_name);
        else
            writer.template write<std::string>("");
    }
    else
    {
        auto sstream = std::stringstream();
        if (marriage.spouse_id.has_value())
            sstream << "배우자: " << marriage.spouse_name << std::endl;

        auto& group_id = ch.group_id();
        if (group_id.has_value())
        {
            auto  guard = ch.server.groups.enter_read(group_id.value());
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
        writer.template write<std::string>(sstream.str());
    }

    writer.template write<uint8_t>(ch.option(OPTION::GROUP));

    auto     cls      = ch.cls();
    auto     level    = ch.level();
    uint64_t remained = 0;
    if (table::ability->contains(cls) && table::ability[cls].contains(level) &&
        table::ability[cls].contains(static_cast<uint8_t>(level + 1)))
    {
        remained = table::ability->stacked_exp(cls, level) - ch.exp();
    }
    writer.template write<uint32_t>(fb::game::encode_client_amount(remained));
    writer.template write<std::string>(table::promotion[ch.cls()][ch.promotion()].name);
}

template <typename Writer>
void write_equip_slot_v651(Writer& writer, const std::shared_ptr<fb::game::equipment>& eq)
{
    if (eq == nullptr)
    {
        writer.template write<uint16_t>(0); // empty look = 0
        writer.template write<uint8_t>(0);
        writer.template write<std::string>("");
        writer.template write<std::string>(""); // unknown_name_b
        writer.template write<uint32_t>(0);     // unknown_u32
    }
    else
    {
        writer.template write<uint16_t>(eq->look());
        writer.template write<uint8_t>(eq->color());
        writer.template write<std::string>(eq->name());
        writer.template write<std::string>("");
        writer.template write<uint32_t>(0);
    }
}

} // namespace

template <CLIENT_VERSION V>
void internal_info<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.template write<uint8_t>(opcode);
    write_internal_info_header<V>(writer, this->ch);

    auto equipments =
        std::array<std::shared_ptr<fb::game::equipment>, 5>{this->ch.items.helmet(),
                                                            this->ch.items.ring(EQUIPMENT_POSITION::LEFT),
                                                            this->ch.items.ring(EQUIPMENT_POSITION::RIGHT),
                                                            this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT),
                                                            this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT)};
    for (auto& eq : equipments)
    {
        if (eq == nullptr)
        {
            writer.template write<uint16_t>(0xFFFF);
            writer.template write<uint8_t>(0x00);
        }
        else
        {
            writer.template write<uint16_t>(eq->look());
            writer.template write<uint8_t>(eq->color());
        }
    }

    writer.template write<uint8_t>(0x00);
    writer.template write<uint8_t>(this->ch.option(OPTION::TRADE));
    writer.template write<uint8_t>(this->ch.option(OPTION::PK_PROTECT));

    writer.template write<uint8_t>((uint8_t)this->ch.achievements.size());
    for (auto& [_, achievement] : this->ch.achievements)
    {
        writer.template write<uint8_t>(achievement->icon);
        writer.template write<uint8_t>(achievement->color);
        writer.template write<std::string>(achievement->text);
    }
    writer.template write<uint8_t>(0x00);
}

template <>
void internal_info<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.template write<uint8_t>(opcode);
    write_internal_info_header<CLIENT_VERSION::v651>(writer, this->ch);

    // parts 1,2,3,4,7,8,20,21,22
    write_equip_slot_v651(writer, this->ch.items.weapon());
    write_equip_slot_v651(writer, this->ch.items.armor());
    write_equip_slot_v651(writer, this->ch.items.shield());
    write_equip_slot_v651(writer, this->ch.items.helmet());
    write_equip_slot_v651(writer, this->ch.items.ring(EQUIPMENT_POSITION::LEFT));
    write_equip_slot_v651(writer, this->ch.items.ring(EQUIPMENT_POSITION::RIGHT));
    write_equip_slot_v651(writer, this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT));
    write_equip_slot_v651(writer, this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT));
    write_equip_slot_v651(writer, nullptr); // parts 22 unknown

    // no fixed 0x00
    writer.template write<uint8_t>(this->ch.option(OPTION::TRADE));
    writer.template write<uint8_t>(this->ch.option(OPTION::PK_PROTECT));

    writer.template write<uint8_t>((uint8_t)this->ch.achievements.size());
    for (auto& [_, achievement] : this->ch.achievements)
    {
        writer.template write<uint8_t>(achievement->icon);
        writer.template write<uint8_t>(achievement->color);
        writer.template write<std::string>(achievement->text);
    }
}

template internal_info<CLIENT_VERSION::v550>::internal_info(fb::game::character&);
template internal_info<CLIENT_VERSION::v565>::internal_info(fb::game::character&);
template internal_info<CLIENT_VERSION::v651>::internal_info(fb::game::character&);
template void internal_info<CLIENT_VERSION::v550>::serialize(fb::stream_writer<big_endian>&) const;
template void internal_info<CLIENT_VERSION::v565>::serialize(fb::stream_writer<big_endian>&) const;
#else
template <CLIENT_VERSION V>
void internal_info<V>::deserialize(fb::stream_reader<big_endian>& reader)
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

    this->equipments.clear();
    for (int i = 0; i < 5; i++)
    {
        equipment_data equip;
        equip.look  = reader.read<uint16_t>();
        equip.color = reader.read<uint8_t>();
        this->equipments.push_back(equip);
    }

    reader.read<uint8_t>();
    this->trade_option      = reader.read<uint8_t>();
    this->pk_protect_option = reader.read<uint8_t>();

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
    reader.read<uint8_t>();
}

template <>
void internal_info<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
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

    this->equipments.clear();
    for (int i = 0; i < 9; i++)
    {
        equipment_data equip;
        equip.look           = reader.read<uint16_t>();
        equip.color          = reader.read<uint8_t>();
        equip.name_a         = reader.read<std::string, uint8_t>();
        equip.unknown_name_b = reader.read<std::string, uint8_t>();
        equip.unknown_u32    = reader.read<uint32_t>();
        this->equipments.push_back(equip);
    }

    this->trade_option      = reader.read<uint8_t>();
    this->pk_protect_option = reader.read<uint8_t>();

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
}

template void internal_info<CLIENT_VERSION::v550>::deserialize(fb::stream_reader<big_endian>&);
template void internal_info<CLIENT_VERSION::v565>::deserialize(fb::stream_reader<big_endian>&);
#endif

} // namespace fb::protocol::game::response
