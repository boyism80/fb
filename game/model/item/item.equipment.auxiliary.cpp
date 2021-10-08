#include "model/item/item.equipment.auxiliary.h"

fb::game::auxiliary::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::auxiliary::master::~master()
{}

fb::game::item::attrs fb::game::auxiliary::master::attr() const
{
    return item::attrs::ITEM_ATTR_AUXILIARY;
}

fb::game::item* fb::game::auxiliary::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::auxiliary(this, dynamic_cast<fb::game::equipment::listener*>(listener));
}



fb::game::auxiliary::auxiliary(const master* master, listener* listener) : 
    equipment(master, listener)
{
}

fb::game::auxiliary::auxiliary(const auxiliary& right) : 
    equipment(right)
{
}

fb::game::auxiliary::~auxiliary()
{
}