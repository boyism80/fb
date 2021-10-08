#include "model/item/item.equipment.armor.h"

fb::game::armor::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::armor::master::~master()
{}

fb::game::item::attrs fb::game::armor::master::attr() const
{
    return item::attrs::ITEM_ATTR_ARMOR;
}

fb::game::item* fb::game::armor::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::armor(this, dynamic_cast<fb::game::equipment::listener*>(listener));
}



fb::game::armor::armor(const master* master, listener* listener) : 
    equipment(master, listener)
{
}

fb::game::armor::armor(const armor& right) : 
    equipment(right)
{
}

fb::game::armor::~armor()
{
}