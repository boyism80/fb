#include <fb/game/item.h>

using namespace fb::game;

helmet::helmet(fb::game::server& server, const fb::model::helmet& model) :
    equipment(server, model)
{ }

helmet::helmet(const helmet& right) :
    equipment(right)
{ }

helmet::~helmet()
{ }