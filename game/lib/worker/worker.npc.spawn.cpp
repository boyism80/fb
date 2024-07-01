#include <fb/game/worker.h>
#include <fb/game/context.h>

fb::game::npc_spawner::npc_spawner(fb::game::context& context) : _context(context)
{ }

fb::generator<fb::game::npc_spawner::input_type> fb::game::npc_spawner::on_ready()
{
    for (auto& [map, spawns] : this->_context.model.npc_spawn)
    {
        for(auto& spawn : spawns)
            co_yield spawn;
    }
}

void fb::game::npc_spawner::on_work(const fb::game::npc_spawner::input_type& value)
{
    auto& model = value.get();
    auto& map = this->_context.maps[model.parent];
    if (map.active)
    {
        auto npc = this->_context.make<fb::game::npc>(this->_context.model.npc[model.npc]);
        npc->map(&map, model.position);
        npc->direction(model.direction);
    }
}

void fb::game::npc_spawner::on_worked(const fb::game::npc_spawner::input_type& input, double percent)
{
     auto& c = fb::console::get();
     c.put("* [%0.2lf%%] NPC 스폰 읽었습니다.", percent);
}

void fb::game::npc_spawner::on_error(const fb::game::npc_spawner::input_type& input, std::exception& e)
{
     auto& c = fb::console::get();
     c.comment("    - %s", e.what());
}

void fb::game::npc_spawner::on_finish()
{
     auto& c = fb::console::get();
     c.next();
}