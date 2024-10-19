#include <item.h>

fb::game::armor::armor(fb::game::context& context, const fb::model::armor& model) : 
    equipment(context, model)
{ }

fb::game::armor::armor(const armor& right) : 
    equipment(right)
{ }

fb::game::armor::~armor()
{ }