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

const fb::model::armor& armor::model() const
{
    return static_cast<const fb::model::armor&>(fb::model::table::item[this->_model_id]);
}