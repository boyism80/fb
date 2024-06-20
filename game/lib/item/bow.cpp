#include <fb/game/item.h>

fb::game::bow::bow(fb::game::context& context, const fb::model::bow& model) : 
    equipment(context, model)
{ }

fb::game::bow::bow(const bow& right) : 
    equipment(right)
{ }

fb::game::bow::~bow()
{ }