#include <fb/game/thread_params.h>

#include <fb/game/server.h>

#include <cstdint>
#include <memory>
#include <utility>

using namespace fb::game;

thread_params::thread_params(server& server) :
    _server(server),
    characters(server)
{ }

void thread_params::add_map(const std::shared_ptr<map>& map)
{
    this->maps[map->id] = map;
    this->load_rezens(map, fb::model::table::mob_spawn.get());
    this->rebuild_map_view();
}

void thread_params::remove_map(uint32_t id)
{
    this->rezens.erase(id);
    this->maps.erase(id);
    this->rebuild_map_view();
}

void thread_params::load_rezens(const std::shared_ptr<map>& map, const rezen::table_ptr& table)
{
    auto loaded = rezen_list();
    auto spawns = table->find(map->model().id);
    for (uint32_t i = 0; spawns != nullptr && i < spawns->size(); i++)
    {
        loaded.push_back(std::make_shared<rezen>(this->_server, table, map->model().id, i, map));
    }
    this->rezens[map->id] = std::move(loaded);
}

void thread_params::rebuild_map_view()
{
    auto view = std::make_shared<map_snapshot>();
    for (auto& [id, map] : this->maps)
    {
        view->push_back(map);
    }
    this->map_view = view;
}
