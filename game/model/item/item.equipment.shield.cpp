#include "model/item/item.equipment.shield.h"

fb::game::shield::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::shield::master::~master()
{}

fb::game::item::attrs fb::game::shield::master::attr() const
{
    return item::attrs::ITEM_ATTR_SHIELD;
}

fb::game::item* fb::game::shield::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::shield(this, dynamic_cast<fb::game::equipment::listener*>(listener));
}



fb::game::shield::shield(const master* master, listener* listener) : 
    equipment(master, listener)
{
}

fb::game::shield::shield(const shield& right) : 
    equipment(right)
{
}

fb::game::shield::~shield()
{
}