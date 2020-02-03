#include "npc.h"

fb::game::npc::core::core(const std::string& name, uint16_t look, uint8_t color) : 
    fb::game::object::core(name, look, color)
{}

fb::game::npc::core::~core()
{}

fb::game::object* fb::game::npc::core::make() const
{
    return new npc(this);
}

fb::ostream fb::game::npc::core::make_dialog_stream(const std::string& message, const std::vector<std::string>& menus, fb::game::map* map) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x01)
        .write_u8(0x00)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write(message, true);

    ostream.write_u16(menus.size());
    for(int i = 0; i < menus.size(); i++)
    {
        ostream.write(menus[i])
            .write_u16(i);
    }

    ostream.write_u8(0x00);
    return ostream;
}

fb::ostream fb::game::npc::core::make_dialog_stream(const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::map* map) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x05)
        .write_u8(0x00)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write(message, true)
        .write_u16(0xFFFF)
        .write_u8(item_slots.size());

    for(auto item_slot : item_slots)
        ostream.write_u8(item_slot);
    ostream.write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::npc::core::make_dialog_stream(const std::string& message, const std::vector<fb::game::item::core*>& cores, fb::game::map* map) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x04)
        .write_u8(0x00)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write(message, true)
        .write_u16(0xFFFF)
        .write_u16(cores.size());

    for(int i = 0; i < cores.size(); i++)
    {
        const auto          item = cores[i];

        ostream.write_u16(item->look())
            .write_u8(item->color())
            .write_u32(item->price())
            .write(item->name())
            .write(item->desc());
    }

    ostream.write_u8(0x00);
    return ostream;
}

fb::ostream fb::game::npc::core::make_input_dialog_stream(const std::string& message, fb::game::map* map) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x03)
        .write_u8(0x00)
        .write_u32(map ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write(message, true)
        .write_u8(0x00);

    return ostream;
}

fb::game::npc::npc(const core* core) : 
    fb::game::object(core)
{
}

fb::game::npc::npc(const npc& right) : 
    object(right)
{
}

fb::game::npc::~npc()
{
}

const std::string& fb::game::npc::script() const
{
    return this->_script;
}

void fb::game::npc::script(const std::string& value)
{
    this->_script = value;
}

fb::game::npc* fb::game::npc::make()
{
    return new npc(*this);
}

fb::ostream fb::game::npc::make_dialog_stream(const std::string& message, const std::vector<std::string>& menus) const
{
    return this->based<npc::core>()->make_dialog_stream(message, menus, this->_map);
}

fb::ostream fb::game::npc::make_dialog_stream(const std::string& message, const std::vector<uint8_t>& item_slots) const
{
    return this->based<npc::core>()->make_dialog_stream(message, item_slots, this->_map);
}

fb::ostream fb::game::npc::make_dialog_stream(const std::string& message, const std::vector<item::core*>& cores, fb::game::map* map) const
{
    return this->based<npc::core>()->make_dialog_stream(message, cores, map);
}

fb::ostream fb::game::npc::make_input_dialog_stream(const std::string& message, fb::game::map* map) const
{
    return this->based<npc::core>()->make_input_dialog_stream(message, this->_map);
}