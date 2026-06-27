#include <fb/game/item.h>

using namespace fb::game;

armor::armor(fb::game::server& server, const fb::model::armor& model, const initial_params& params) :
    equipment(server, model, params)
{ }

armor::armor(const armor& right) :
    equipment(right)
{ }

armor::~armor()
{ }