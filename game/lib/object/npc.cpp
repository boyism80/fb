#include <fb/game/context.h>
#include <fb/game/npc.h>

using namespace fb::game;

npc::npc(fb::game::context& context, const fb::model::npc& model) :
    fb::game::object(context, model, initial_params())
{ }

npc::npc(const npc& right) :
    object(right)
{ }

npc::~npc()
{ }

void npc::assert_thread() const
{
    if (this->_map != nullptr)
        object::assert_thread();
    else
        return;
}