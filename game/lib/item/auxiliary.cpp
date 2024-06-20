#include <fb/game/item.h>

fb::game::auxiliary::auxiliary(fb::game::context& context, const fb::model::auxiliary& model) : 
    equipment(context, model)
{ }

fb::game::auxiliary::auxiliary(const auxiliary& right) : 
    equipment(right)
{ }

fb::game::auxiliary::~auxiliary()
{ }