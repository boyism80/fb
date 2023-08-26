#include <fb/game/item.h>

fb::game::bow::model::model(const fb::game::equipment::model::config& config) : fb::game::equipment::model(config)
{ }

fb::game::bow::model::~model()
{ }

fb::game::item::attrs fb::game::bow::model::attr() const
{
    return item::attrs::ARROW;
}


fb::game::bow::bow(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::bow::bow(const bow& right) : 
    equipment(right)
{ }

fb::game::bow::~bow()
{ }