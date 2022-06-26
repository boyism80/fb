#include <fb/game/item.h>

fb::game::helmet::master::master(const fb::game::equipment::master::config& config) : fb::game::equipment::master(config)
{ }

fb::game::helmet::master::~master()
{ }

fb::game::item::attrs fb::game::helmet::master::attr() const
{
    return item::attrs::HELMET;
}


fb::game::helmet::helmet(fb::game::context& context, const master* master) : 
    equipment(context, master)
{ }

fb::game::helmet::helmet(const helmet& right) : 
    equipment(right)
{ }

fb::game::helmet::~helmet()
{ }