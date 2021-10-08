#include "model/item/item.equipment.helmet.h"

fb::game::helmet::master::master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color) : 
    fb::game::equipment::master(id, name, look, dress, color)
{}

fb::game::helmet::master::~master()
{}

fb::game::item::attrs fb::game::helmet::master::attr() const
{
    return item::attrs::ITEM_ATTR_HELMET;
}

fb::game::item* fb::game::helmet::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::helmet(this, dynamic_cast<fb::game::equipment::listener*>(listener));
}



fb::game::helmet::helmet(const master* master, listener* listener) : 
    equipment(master, listener)
{
}

fb::game::helmet::helmet(const helmet& right) : 
    equipment(right)
{
}

fb::game::helmet::~helmet()
{
}