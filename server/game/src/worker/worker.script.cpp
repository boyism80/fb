#include <fb/game/server.h>
#include <fb/game/worker.h>
#include <filesystem>
#include <format>
#include <mutex>
#include <set>

using table = fb::model::table;

fb::game::script_loader::script_loader(fb::game::server& server) :
    fb::parallel_worker<script_work>(server),
    _server(server)
{ }

fb::generator<fb::game::script_work> fb::game::script_loader::on_ready()
{
    fb::console::progress("Loading script files", 0);

    auto scripts = std::vector<std::string>{};
    scripts.push_back("scripts/server.lua");
    scripts.push_back("scripts/interaction.lua");
    scripts.push_back("scripts/script.lua");
    // scripts/init.lua is executed once at end of startup via server::init_script()
    // (coroutine-aware); do not dump/pcall it here.
    scripts.push_back("scripts/schedule/schedule.lua");
    scripts.push_back(fb::model::const_value::script::F1_EVENT_SCRIPT);
    scripts.push_back(fb::model::const_value::script::F2_EVENT_SCRIPT);

    auto spell_table = table::spell;
    for (auto& [id, v] : spell_table)
    {
        auto path = std::format("scripts/spell/{}.lua", id);
        if (std::filesystem::exists(path))
            scripts.push_back(path);
    }

    auto item_table = table::item;
    for (auto& [id, v] : item_table)
    {
        auto path = std::format("scripts/item/{}.lua", id);
        if (std::filesystem::exists(path))
            scripts.push_back(path);
    }

    auto npc_table = table::npc;
    for (auto& [id, v] : npc_table)
    {
        auto path = std::format("scripts/npc/{}.lua", id);
        if (std::filesystem::exists(path))
            scripts.push_back(path);
    }

    auto host       = fb::config<uint8_t>("id");
    auto warp_table = table::warp;
    for (auto& [k, v] : warp_table)
    {
        auto  map_table2 = table::map;
        auto& map        = map_table2[k];
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

    auto mob_table = table::mob;
    for (auto& [id, v] : mob_table)
    {
        auto path = std::format("scripts/mob/{}.lua", id);
        if (std::filesystem::exists(path))
            scripts.push_back(path);
    }

    auto map_table = table::map;
    for (auto& [id, v] : map_table)
    {
        auto path = std::format("scripts/map/{}.lua", id);
        if (std::filesystem::exists(path))
            scripts.push_back(path);
    }

    for (auto& [_, root] : _server.lua)
    {
        co_yield script_work{root.get(), scripts};
    }
}

async::task<void> fb::game::script_loader::on_work(const script_work& work)
{
    auto& thread  = work.root->initial_thread();
    auto  builder = thread.new_builder<void>();
    builder.func  = [root = work.root, scripts = work.scripts](auto&) -> async::task<void> {
        static auto logs  = std::set<std::string>{};
        static auto mutex = std::mutex{};

        for (auto& script : scripts)
        {
            if (root->dump(script))
                continue;

            auto message = std::format("cannot load script {}", script);
            auto _       = std::lock_guard(mutex);
            if (logs.contains(message) == false)
            {
                logs.insert(message);
                fb::console::comment("    - {}", message);
            }
        }
        co_return;
    };
    co_await builder.dispatch();
}

void fb::game::script_loader::on_worked(const script_work& input, double percent)
{
    fb::console::progress("Loading script files", percent);
}

void fb::game::script_loader::on_error(const script_work& input, std::exception& e)
{
    fb::console::comment("    - {}", e.what());
}

void fb::game::script_loader::on_finish()
{
    fb::console::progress("Loading script files", 100.f);
    fb::console::newline();
}
