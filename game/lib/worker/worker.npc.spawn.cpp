#include <context.h>
#include <worker.h>

fb::game::npc_spawner::npc_spawner(fb::game::context& context) :
    _context(context)
{ }

fb::generator<fb::game::npc_spawner::input_type> fb::game::npc_spawner::on_ready()
{
    for (auto& [map, spawns] : this->_context.model.npc_spawn)
    {
        for (auto& spawn : spawns)
            co_yield spawn;
    }
}

void fb::game::npc_spawner::on_work(const fb::game::npc_spawner::input_type& value)
{
    auto& spawn_model = value.get();
    auto& npc_model   = this->_context.model.npc[spawn_model.npc];
    auto& map_model   = this->_context.model.map[spawn_model.parent];
    if (this->_context.maps.contains(spawn_model.parent) == false)
        throw std::runtime_error(
            std::format("NPC {}를 배치할 수 없습니다. {} 맵이 로드되지 않았습니다.", npc_model.name, map_model.name));

    auto& map = this->_context.maps[spawn_model.parent];
    if (map.active == false)
        return;

    auto thread = this->_context.thread(map);
    if (thread == nullptr)
        throw std::runtime_error("thread exception");

    this->_context.threads.enqueue(*npc,, [this, &map, &spawn_model]() -> async::task<void> {
        auto npc = this->_context.make<fb::game::npc>(this->_context.model.npc[spawn_model.npc]);
        std::ignore = co_await npc->map(&map, spawn_model.position);
        std::ignore = co_await npc->direction(spawn_model.direction);
    });
}

void fb::game::npc_spawner::on_worked(const fb::game::npc_spawner::input_type& input, double percent)
{
    fb::console::put("* [{:0.2f}%] NPC 스폰 읽었습니다.", percent);
}

void fb::game::npc_spawner::on_error(const fb::game::npc_spawner::input_type& input, std::exception& e)
{
    fb::console::comment("    - {}", e.what());
}

void fb::game::npc_spawner::on_finish()
{
    fb::console::newline();
}