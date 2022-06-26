#include <fb/game/item.h>

fb::game::armor::master::master(const fb::game::equipment::master::config& config) : fb::game::equipment::master(config)
{ }

fb::game::armor::master::~master()
{ }

fb::game::item::attrs fb::game::armor::master::attr() const
{
    return item::attrs::ARMOR;
}


fb::game::armor::armor(fb::game::context& context, const master* master) : 
    equipment(context, master)
{ }

fb::game::armor::armor(const armor& right) : 
    equipment(right)
{ }

fb::game::armor::~armor()
{ }