#include <fb/game/item.h>

fb::game::shield::model::model(const fb::game::equipment::model::config& config) : fb::game::equipment::model(config)
{ }

fb::game::shield::model::~model()
{ }

fb::game::item::attrs fb::game::shield::model::attr() const
{
    return item::attrs::SHIELD;
}


fb::game::shield::shield(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::shield::shield(const shield& right) : 
    equipment(right)
{ }

fb::game::shield::~shield()
{ }