#include <fb/game/context.h>
#include <fb/game/npc.h>

using namespace fb::game;

npc::npc(fb::game::context& context, const fb::model::npc& model) :
    fb::game::object(context, model, initial_params()),
    _next_soliloquy(fb::model::datetime() + fb::model::timespan(0, 0, 0, random<uint16_t>(60, 120), 0))
{ }

npc::npc(const npc& right) :
    object(right)
{ }

npc::~npc()
{ }

void fb::game::npc::soliloquy()
{
    auto& model = this->based<fb::model::npc>();
    if (this->context.model.soliloquy.contains(model.id) == false)
        return;

    auto now = fb::model::datetime();
    if (now < this->_next_soliloquy)
        return;

    auto& soliloquy = this->context.model.soliloquy[model.id];
    auto  i         = random<uint16_t>(0, soliloquy.size());
    this->chat(soliloquy[i].message);
    this->_next_soliloquy = now;
    this->_next_soliloquy.add_seconds(random<uint16_t>(60, 120));
}

void npc::assert_thread() const
{
    if (this->_map != nullptr)
        object::assert_thread();
    else
        return;
}