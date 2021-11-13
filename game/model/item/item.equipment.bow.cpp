#include "model/item/item.equipment.bow.h"

fb::game::bow::master::master(const fb::game::equipment::master::config& config) : fb::game::equipment::master(config)
{ }

fb::game::bow::master::~master()
{ }

fb::game::item::attrs fb::game::bow::master::attr() const
{
    return item::attrs::ARROW;
}


fb::game::bow::bow(fb::game::context& context, const master* master) : 
    equipment(context, master)
{ }

fb::game::bow::bow(const bow& right) : 
    equipment(right)
{ }

fb::game::bow::~bow()
{ }