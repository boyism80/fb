#include <fb/game/server.h>
#include <fb/game/worker.h>
#include <fb/model/model.h>
#include <macro.h>

using table = fb::model::table;

fb::game::npc_spawner::npc_spawner(fb::game::server& server) :
    fb::parallel_worker<input_type>(server),
    _server(server)
{ }

fb::generator<fb::game::npc_spawner::input_type> fb::game::npc_spawner::on_ready()
{
    fb::console::progress("Loading npc spawns", 0);

    for (auto& [map, spawns] : table::npc_spawn)
    {
        for (auto& spawn : spawns)
        {
            co_yield spawn;
        }
    }
}

async::task<void> fb::game::npc_spawner::on_work(const fb::game::npc_spawner::input_type& value)
{
    auto& spawn_model = value.get();
    auto& npc_model   = table::npc[spawn_model.npc];
    auto& map_model   = table::map[spawn_model.parent];
    if (this->_server.maps.contains(spawn_model.parent) == false)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_ASSET_NPC_SPAWN_FAILED), npc_model.name, map_model.name));

    auto map = this->_server.maps[spawn_model.parent];
    if (map == nullptr || map->active == false)
        co_return;

    if (map->loaded() == false)
        co_return;

    this->_server.maps.spawn_npc(spawn_model);
    co_return;
}

void fb::game::npc_spawner::on_worked(const fb::game::npc_spawner::input_type& input, double percent)
{
    fb::console::progress("Loading npc spawns", percent);
}

void fb::game::npc_spawner::on_error(const fb::game::npc_spawner::input_type& input, std::exception& e)
{
    fb::console::comment("    - {}", e.what());
}

void fb::game::npc_spawner::on_finish()
{
    fb::console::progress("Loading npc spawns", 100.f);
    fb::console::newline();
}
