#include <fb/game/item.h>

fb::game::armor::model::model(const fb::game::equipment::model::config& config) : fb::game::equipment::model(config)
{ }

fb::game::armor::model::~model()
{ }

fb::game::item::ATTRIBUTE fb::game::armor::model::attr() const
{
    return item::ATTRIBUTE::ARMOR;
}


fb::game::armor::armor(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::armor::armor(const armor& right) : 
    equipment(right)
{ }

fb::game::armor::~armor()
{ }