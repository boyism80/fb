#include <fb/game/item.h>

fb::game::auxiliary::model::model(const fb::game::equipment::model::config& config) : fb::game::equipment::model(config)
{ }

fb::game::auxiliary::model::~model()
{ }

fb::game::item::ATTRIBUTE fb::game::auxiliary::model::attr() const
{
    return item::ATTRIBUTE::AUXILIARY;
}


fb::game::auxiliary::auxiliary(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::auxiliary::auxiliary(const auxiliary& right) : 
    equipment(right)
{ }

fb::game::auxiliary::~auxiliary()
{ }