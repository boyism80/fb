#include "model/item/item.equipment.bow.h"

fb::game::bow::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::bow::master::~master()
{}

fb::game::item::attrs fb::game::bow::master::attr() const
{
    return item::attrs::ITEM_ATTR_ARROW;
}

fb::game::item* fb::game::bow::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::bow(this, dynamic_cast<fb::game::equipment::listener*>(listener));
}



fb::game::bow::bow(const master* master, listener* listener) : 
    equipment(master, listener)
{
}

fb::game::bow::bow(const bow& right) : 
    equipment(right)
{
}

fb::game::bow::~bow()
{
}