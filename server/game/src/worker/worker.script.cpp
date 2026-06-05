#include <fb/game/server.h>
#include <fb/game/worker.h>

using table = fb::model::table;

fb::game::script_loader::script_loader(fb::game::server& server) :
    _server(server)
{ }

fb::generator<std::function<async::task<void>()>> fb::game::script_loader::on_ready()
{
    fb::console::progress("Loading script files", 0);

    auto scripts = std::vector<std::string>{};
    scripts.push_back("scripts/server.lua");
    scripts.push_back("scripts/spell.lua");
    scripts.push_back("scripts/npc.lua");
    scripts.push_back("scripts/interaction.lua");
    scripts.push_back("scripts/command.lua");
    scripts.push_back("scripts/script.lua");
    scripts.push_back("scripts/init.lua");
    scripts.push_back(fb::model::const_value::script::F1_EVENT_SCRIPT);
    scripts.push_back(fb::model::const_value::script::F2_EVENT_SCRIPT);

    for (auto& [k, v] : table::spell)
    {
        if (v.script.empty())
            continue;

        scripts.push_back(v.script);
    }

    for (auto& [k, v] : table::item)
    {
        if (v.script.empty())
            continue;

        scripts.push_back(v.script);
    }

    for (auto& [k, v] : table::npc)
    {
        if (v.script.empty())
            continue;

        scripts.push_back(v.script);
    }

    auto host = fb::config<uint8_t>("id");
    for (auto& [k, v] : table::warp)
    {
        auto& map = table::map[k];
        if (map.host != host)
            continue;

        for (auto& warp : v)
        {
            if (warp.dest.header != DSL::script)
                continue;

            auto params = fb::model::dsl::script(warp.dest.params);
            scripts.push_back(params.path);
        }
    }

    for (auto& [k, v] : table::mob)
    {
        if (v.script.empty())
            continue;

        scripts.push_back(v.script);
    }

    static auto& ist   = fb::lua::context_pool::ist();
    static auto  logs  = std::set<std::string>{};
    static auto  mutex = std::mutex{};
    for (auto& [_, root] : ist)
    {
        auto& thread = root->initial_thread();
        co_yield [&thread, &root, scripts]() -> async::task<void> {
            auto builder = thread.new_builder<void>();
            builder.func = [&root, scripts](auto&) -> async::task<void> {
                for (auto& script : scripts)
                {
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
                }
                co_return;
            };
            co_return co_await builder.dispatch();
        };
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