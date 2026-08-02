#include <fb/game/item.h>

using namespace fb::game;

helmet::helmet(fb::game::server& server, const fb::model::helmet& model, const initial_params& params) :
    equipment(server, model, params)
{ }

helmet::helmet(const helmet& right) :
    equipment(right)
{ }

helmet::~helmet()
{ }

const fb::model::helmet& helmet::model() const
{
    return static_cast<const fb::model::helmet&>(fb::model::table::item[this->_model_id]);
}