#include <context.h>
#include <worker.h>

fb::game::npc_spawner::npc_spawner(fb::game::context& context) :
    _context(context)
{ }

fb::generator<fb::game::npc_spawner::input_type> fb::game::npc_spawner::on_ready()
{
    for (auto& [map, spawns] : this->_context.model.npc_spawn)
    {
        if (this->_context.maps[map].active == false)
            continue;

        for (auto& spawn : spawns)
            co_yield spawn;
    }
}

void fb::game::npc_spawner::on_work(const fb::game::npc_spawner::input_type& value)
{
    auto& model  = value.get();
    auto& map    = this->_context.maps[model.parent];
    auto  thread = this->_context.thread(map);
    if (thread == nullptr)
        throw std::runtime_error("thread exception");

    auto task = thread->dispatch([this, &model, &map]() -> async::task<void> {
        auto npc = this->_context.make<fb::game::npc>(this->_context.model.npc[model.npc]);
        async::awaitable_get(npc->map(&map, model.position));
        npc->direction(model.direction);
        co_return;
    });
    async::awaitable_get(task);
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
    fb::console::next();
}