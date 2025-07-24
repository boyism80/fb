#include <fb/game/server.h>
#include <fb/game/worker.h>

fb::game::script_loader::script_loader(fb::game::server& server) :
    _server(server)
{ }

fb::generator<std::function<async::task<void>()>> fb::game::script_loader::on_ready()
{
    auto scripts = std::set<std::string>{};
    scripts.insert("scripts/spell.lua");
    scripts.insert("scripts/npc.lua");
    scripts.insert("scripts/interaction.lua");
    scripts.insert("scripts/command.lua");
    scripts.insert("scripts/script.lua");

    for (auto& [k, v] : this->_server.model.spell)
    {
        if (v.script.empty())
            continue;

        scripts.insert(v.script);
    }

    for (auto& [k, v] : this->_server.model.item)
    {
        if (v.script.empty())
            continue;

        scripts.insert(v.script);
    }

    for (auto& [k, v] : this->_server.model.npc)
    {
        if (v.script.empty())
            continue;

        scripts.insert(v.script);
    }

    auto host = fb::config<uint8_t>("id");
    for (auto& [k, v] : this->_server.model.warp)
    {
        auto& map = this->_server.model.map[k];
        if (map.host != host)
            continue;

        for (auto& warp : v)
        {
            if (warp.dest.header != DSL::script)
                continue;

            auto params = fb::model::dsl::script(warp.dest.params);
            scripts.insert(params.path);
        }
    }

    for (auto& [k, v] : this->_server.model.mob)
    {
        if (v.script.empty())
            continue;

        scripts.insert(v.script);
    }

    static auto& ist   = fb::lua::context_pool::ist();
    static auto  logs  = std::set<std::string>{};
    static auto  mutex = std::mutex{};
    for (auto& [_, root] : ist)
    {
        auto& thread = root->initial_thread();
        for (auto& script : scripts)
        {
            co_yield [&thread, root, script]() -> async::task<void> {
                co_await thread.dispatch([root, script](auto&) -> async::task<void> {
                    try
                    {
                        root->dump(script);
                    }
                    catch (std::exception& e)
                    {
                        auto _ = std::lock_guard(mutex);
                        if (logs.contains(e.what()) == false)
                        {
                            logs.insert(e.what());
                            fb::console::comment("    - {}", e.what());
                        }
                    }
                    co_return;
                });
            };
        }
    }
}

void fb::game::script_loader::on_work(const std::function<async::task<void>()>& value)
{
    async::awaitable_get(value());
}

void fb::game::script_loader::on_worked(const std::function<async::task<void>()>& input, double percent)
{
    fb::console::progress("Loading script files", percent);
}

void fb::game::script_loader::on_error(const std::function<async::task<void>()>& input, std::exception& e)
{
    fb::console::comment("    - {}", e.what());
}

void fb::game::script_loader::on_finish()
{
    fb::console::newline();
}