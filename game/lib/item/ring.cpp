#include <fb/game/item.h>

fb::game::ring::ring(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::ring::ring(const ring& right) : 
    equipment(right)
{ }

fb::game::ring::~ring()
{ }