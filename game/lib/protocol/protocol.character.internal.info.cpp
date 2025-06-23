#include <fb/game/protocol/character/internal_info.h>
#ifndef BOT
#include <fb/game/group.h>
#endif

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> internal_info::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<int8_t>(this->ch.phydef());
    writer.write<uint8_t>(this->ch.dam());
    writer.write<uint8_t>(this->ch.hit());

    auto& clan_lock_ptr = this->ch.clan();
    if (clan_lock_ptr != nullptr)
    {
        clan_lock_ptr->read([&writer](auto& clan) {
            writer.write<std::string>(clan.name());
            writer.write<std::string>(clan.title().value_or(""));
        });
    }
    else
    {
        writer.write<std::string>("");
        writer.write<std::string>("");
    }
    writer.write<std::string>(this->ch.title());

    auto& shared_group_lock = this->ch.group();
    if (shared_group_lock != nullptr)
    {
        shared_group_lock->read([&writer](auto& group) {
            if (group.inited())
            {
                auto sstream = std::stringstream();
                sstream << "그룹원" << std::endl << "  * " << group.master() << std::endl;

                for (auto& member : group.members())
                    sstream << "    " << member << std::endl;
                writer.write<std::string>(sstream.str());
            }
            else
            {
                writer.write<std::string>("그룹 정보 가져오는중.");
            }
        });
    }
    else
    {
        writer.write<std::string>("그룹 없음.");
    }
    writer.write<uint8_t>(this->ch.option(OPTION::GROUP));

    uint32_t remained_exp = this->ch.experience_remained();
    writer.write<uint32_t>(remained_exp);

    auto& class_name = model.promotion[this->ch.cls()][this->ch.promotion()].name;
    writer.write<std::string>(class_name);

    std::shared_ptr<fb::game::equipment> equipments[] = {this->ch.items.helmet(),
                                                         this->ch.items.ring(EQUIPMENT_POSITION::LEFT),
                                                         this->ch.items.ring(EQUIPMENT_POSITION::RIGHT),
                                                         this->ch.items.auxiliary(EQUIPMENT_POSITION::LEFT),
                                                         this->ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT)};
    for (int i = 0, size = sizeof(equipments) / sizeof(fb::game::equipment*); i < size; i++)
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
        auto& model = achievement->model;
        writer.write<uint8_t>(model.look);
        writer.write<uint8_t>(model.color);
        writer.write<std::string>(achievement->text.value_or(model.text));
    }
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> internal_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->phydef       = reader.read<int8_t>();
    this->dam          = reader.read<uint8_t>();
    this->hit          = reader.read<uint8_t>();
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

} // namespace fb::protocol::game::response
