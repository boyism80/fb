#include "model/item/item.equipment.shield.h"

fb::game::shield::master::master(const fb::game::equipment::master::config& config) : fb::game::equipment::master(config)
{ }

fb::game::shield::master::~master()
{ }

fb::game::item::attrs fb::game::shield::master::attr() const
{
    return item::attrs::SHIELD;
}


fb::game::shield::shield(fb::game::context& context, const master* master) : 
    equipment(context, master)
{ }

fb::game::shield::shield(const shield& right) : 
    equipment(right)
{ }

fb::game::shield::~shield()
{ }