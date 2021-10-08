#include "model/item/item.equipment.ring.h"

fb::game::ring::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::ring::master::~master()
{}

fb::game::item::attrs fb::game::ring::master::attr() const
{
    return item::attrs::ITEM_ATTR_RING;
}

fb::game::item* fb::game::ring::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::ring(this, dynamic_cast<fb::game::equipment::listener*>(listener));
}



fb::game::ring::ring(const master* master, listener* listener) : 
    equipment(master, listener)
{
}

fb::game::ring::ring(const ring& right) : 
    equipment(right)
{
}

fb::game::ring::~ring()
{
}