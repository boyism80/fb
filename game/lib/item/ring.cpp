#include <fb/game/item.h>

fb::game::ring::model::model(const fb::game::equipment::model::config& config) : fb::game::equipment::model(config)
{ }

fb::game::ring::model::~model()
{ }

fb::game::item::attrs fb::game::ring::model::attr() const
{
    return item::attrs::RING;
}


fb::game::ring::ring(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::ring::ring(const ring& right) : 
    equipment(right)
{ }

fb::game::ring::~ring()
{ }