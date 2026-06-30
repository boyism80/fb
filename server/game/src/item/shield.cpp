#include <fb/game/item.h>

using namespace fb::game;

shield::shield(fb::game::server& server, const fb::model::shield& model, const initial_params& params) :
    equipment(server, model, params)
{ }

shield::shield(const shield& right) :
    equipment(right)
{ }

shield::~shield()
{ }