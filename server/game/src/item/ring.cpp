#include <fb/game/item.h>

using namespace fb::game;

ring::ring(fb::game::server& server, const fb::model::ring& model) :
    equipment(server, model)
{ }

ring::ring(const ring& right) :
    equipment(right)
{ }

ring::~ring()
{ }