#include <fb/game/thread_params.h>
#include <fb/game/server.h>

using namespace fb::game;

thread_params::thread_params(server& server) :
    characters(server)
{ }

void thread_params::add_map(const std::shared_ptr<map>& map)
{
    this->maps[map->id] = map;
    this->rebuild_map_view();
}

void thread_params::remove_map(uint32_t id)
{
    this->maps.erase(id);
    this->rebuild_map_view();
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
