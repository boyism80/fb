#include <item.h>

fb::game::ring::ring(fb::game::context& context, const fb::model::ring& model) : 
    equipment(context, model)
{ }

fb::game::ring::ring(const ring& right) : 
    equipment(right)
{ }

fb::game::ring::~ring()
{ }