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

fb::ostream fb::game::npc::core::make_dialog_stream(const std::string& message, const std::vector<fb::game::item::core*>& items, fb::game::map* map) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x2F)
        .write_u8(0x04)
        .write_u8(0x0F)
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
        .write_u16(items.size());

    for(int i = 0; i < items.size(); i++)
    {
        const auto          item = items[i];
        ostream.write_u16(item->look())
            .write_u8(item->color())
            .write_u32(item->price())
            .write(item->name())
            .write(item->desc());
    }
    
    ostream.write_u8(0x00);
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

fb::game::object::types fb::game::npc::type() const
{
    return static_cast<const core*>(this->_core)->type();
}

fb::ostream fb::game::npc::make_dialog_stream(const std::string& message, const std::vector<fb::game::item::core*>& items) const
{
    return this->based<npc::core>()->make_dialog_stream(message, items, this->_map);
}
