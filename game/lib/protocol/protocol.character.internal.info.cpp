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
        clan_lock_ptr->lock([&writer](auto& clan) {
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
        shared_group_lock->lock([&writer](auto& group) {
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

    fb::game::equipment* equipments[] = {this->ch.items.helmet(),
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

    writer.write<uint8_t>((uint8_t)this->ch.traces.size());
    for (auto& [_, trace] : this->ch.traces)
    {
        auto& model = trace->model;
        writer.write<uint8_t>(model.look);
        writer.write<uint8_t>(model.color);
        writer.write<std::string>(trace->text.value_or(model.text));
    }
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> internal_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
