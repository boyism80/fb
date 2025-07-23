#include <fb/game/item.h>

using namespace fb::game;

auxiliary::auxiliary(fb::game::server& server, const fb::model::auxiliary& model) :
    equipment(server, model)
{ }

auxiliary::auxiliary(const auxiliary& right) :
    equipment(right)
{ }

auxiliary::~auxiliary()
{ }