#include <fb/game/item.h>

fb::game::shield::shield(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::shield::shield(const shield& right) : 
    equipment(right)
{ }

fb::game::shield::~shield()
{ }