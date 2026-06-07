#include <fb/game/service/schedule.h>
#include <fb/game/server.h>
#include <fb/lua.h>
#include <fb/logger.h>
#include <format>

using namespace fb::game;
using table = fb::model::table;

service::schedule::schedule(fb::game::server& server) :
    server(server)
{ }

void service::schedule::init()
{
    auto now = this->server.now();

    for (const auto& entry : table::schedule)
    {
        auto next = entry.next_execution(now);
        if (next.has_value())
            this->_tasks[entry.id] = next.value();
    }
}

async::task<void> service::schedule::poll()
{
    auto* logic_thread = this->server.threads.least_loaded();
    if (logic_thread == nullptr)
    {
        fb::logger::warn("No logic thread available for schedule execution");
        co_return;
    }

    co_await logic_thread->switching();

    auto now       = this->server.now();
    auto to_remove = std::vector<uint32_t>{};

    for (auto& [schedule_id, next_execution] : this->_tasks)
    {
        if (now < next_execution)
            continue;

        const auto& entry = table::schedule[schedule_id];

        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
            try
            {
                lua->load(entry.script);
                lua->func(entry.func);
                std::ignore = co_await lua->call(0);
            }
            catch (std::exception& e)
            {
                fb::logger::warn(std::format("Schedule {} script execution failed: {}", schedule_id, e.what()));
            }
        }

        if (entry.repeat.has_value())
        {
            auto next_time = next_execution + entry.repeat.value();

            if (entry.date.end.has_value() && next_time > entry.date.end.value())
                to_remove.push_back(schedule_id);
            else
                next_execution = next_time;
        }
        else
        {
            to_remove.push_back(schedule_id);
        }
    }

    for (auto schedule_id : to_remove)
        this->_tasks.erase(schedule_id);

    co_return;
}
