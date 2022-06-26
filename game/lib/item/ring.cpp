#include <fb/game/item.h>

fb::game::ring::master::master(const fb::game::equipment::master::config& config) : fb::game::equipment::master(config)
{ }

fb::game::ring::master::~master()
{ }

fb::game::item::attrs fb::game::ring::master::attr() const
{
    return item::attrs::RING;
}


fb::game::ring::ring(fb::game::context& context, const master* master) : 
    equipment(context, master)
{ }

fb::game::ring::ring(const ring& right) : 
    equipment(right)
{ }

fb::game::ring::~ring()
{ }