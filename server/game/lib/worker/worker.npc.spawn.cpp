#include <fb/game/server.h>
#include <fb/game/worker.h>

using table = fb::model::table;

fb::game::npc_spawner::npc_spawner(fb::game::server& server) :
    _server(server)
{ }

fb::generator<fb::game::npc_spawner::input_type> fb::game::npc_spawner::on_ready()
{
    for (auto& [map, spawns] : table::npc_spawn)
    {
        for (auto& spawn : spawns)
            co_yield spawn;
    }
}

void fb::game::npc_spawner::on_work(const fb::game::npc_spawner::input_type& value)
{
    auto& spawn_model = value.get();
    auto& npc_model   = table::npc[spawn_model.npc];
    auto& map_model   = table::map[spawn_model.parent];
    if (this->_server.maps.contains(spawn_model.parent) == false)
        throw std::runtime_error(std::format("NPC {}를 배치할 수 없습니다. {} 맵이 로드되지 않았습니다.", npc_model.name, map_model.name));

    auto map = this->_server.maps[spawn_model.parent];
    if (map == nullptr || map->active == false)
        return;

    auto thread = map->thread();
    if (thread == nullptr)
        throw std::runtime_error("thread exception");

    // Use smart pointer for NPC creation
    auto& model = table::npc[spawn_model.npc];
    auto  npc   = this->_server.make<fb::game::npc>(model);
    auto  weak  = npc->weak_from_this_as<fb::game::npc>();
    auto  fn    = [](std::shared_ptr<fb::game::npc> npc, std::shared_ptr<fb::game::map> map, fb::model::npc_spawn& spawn_model) -> async::task<void> {
        std::ignore = co_await npc->map(map, spawn_model.position);
        npc->direction(spawn_model.direction);
    };
    this->_server.threads.enqueue(weak, [fn, npc, map, &spawn_model](auto&) -> async::task<void> {
        co_await fn(npc, map, spawn_model);
    });
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
    fb::console::newline();
}