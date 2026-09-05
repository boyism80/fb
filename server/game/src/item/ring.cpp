#include <fb/game/item.h>

using namespace fb::game;

ring::ring(fb::game::server& server, const fb::model::ring& model, const initial_params& params) :
    equipment(server, model, params)
{ }

ring::ring(const ring& right) :
    equipment(right)
{ }

ring::~ring()
{ }

const fb::model::ring& ring::model() const
{
    return static_cast<const fb::model::ring&>(fb::model::table::item[this->_model_id]);
}