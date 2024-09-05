#include <fb/game/item.h>

fb::game::helmet::helmet(fb::game::context& context, const fb::model::helmet& model) : 
    equipment(context, model)
{ }

fb::game::helmet::helmet(const helmet& right) : 
    equipment(right)
{ }

fb::game::helmet::~helmet()
{ }