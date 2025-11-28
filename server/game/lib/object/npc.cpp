#include <fb/game/server.h>
#include <fb/game/npc.h>

using namespace fb::game;
using table = fb::model::table;

npc::npc(fb::game::server& server, const fb::model::npc& model) :
    fb::game::object(server, model, initial_params()),
    _soliloquy_time(random<uint16_t>(60, 120))
{ }

npc::npc(const npc& right) :
    object(right)
{ }

npc::~npc()
{ }

void fb::game::npc::soliloquy()
{
    auto& model = this->based<fb::model::npc>();
    if (table::soliloquy.contains(model.id) == false)
        return;

    this->_soliloquy_time--;
    if (this->_soliloquy_time > 0)
        return;

    auto& soliloquy = table::soliloquy[model.id];
    auto  i         = random<uint16_t>(0, soliloquy.size() - 1);
    this->chat(soliloquy[i].message);
    this->_soliloquy_time = random<uint16_t>(60, 120);
}

void npc::assert_thread() const
{
    if (this->_map != nullptr)
        object::assert_thread();
    else
        return;
}