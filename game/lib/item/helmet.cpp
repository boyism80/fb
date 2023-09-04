#include <fb/game/item.h>

fb::game::helmet::model::model(const fb::game::equipment::model::config& config) : fb::game::equipment::model(config)
{ }

fb::game::helmet::model::~model()
{ }

fb::game::item::attrs fb::game::helmet::model::attr() const
{
    return item::attrs::HELMET;
}


fb::game::helmet::helmet(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::helmet::helmet(const helmet& right) : 
    equipment(right)
{ }

fb::game::helmet::~helmet()
{ }