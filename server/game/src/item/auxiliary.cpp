#include <fb/game/item.h>

using namespace fb::game;

auxiliary::auxiliary(fb::game::server& server, const fb::model::auxiliary& model, const initial_params& params) :
    equipment(server, model, params)
{ }

auxiliary::auxiliary(const auxiliary& right) :
    equipment(right)
{ }

auxiliary::~auxiliary()
{ }

const fb::model::auxiliary& auxiliary::model() const
{
    return static_cast<const fb::model::auxiliary&>(fb::model::table::item[this->_model_id]);
}