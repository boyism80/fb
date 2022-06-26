#include <fb/game/item.h>

fb::game::auxiliary::master::master(const fb::game::equipment::master::config& config) : fb::game::equipment::master(config)
{ }

fb::game::auxiliary::master::~master()
{ }

fb::game::item::attrs fb::game::auxiliary::master::attr() const
{
    return item::attrs::AUXILIARY;
}


fb::game::auxiliary::auxiliary(fb::game::context& context, const master* master) : 
    equipment(context, master)
{ }

fb::game::auxiliary::auxiliary(const auxiliary& right) : 
    equipment(right)
{ }

fb::game::auxiliary::~auxiliary()
{ }