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

    uint32_t index          = 0;
    auto     schedule_table = table::schedule;
    for (const auto& entry : schedule_table)
    {
        auto next = entry.next_execution(now);
        if (next.has_value())
            this->_tasks[index] = next.value();
        ++index;
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

    for (auto& [schedule_index, next_execution] : this->_tasks)
    {
        if (now < next_execution)
            continue;

        if (schedule_index >= table::schedule.size())
        {
            fb::logger::warn(std::format("Schedule index {} out of range", schedule_index));
            to_remove.push_back(schedule_index);
            continue;
        }

        const auto& entry = table::schedule[schedule_index];
        if (entry.script.empty() || entry.func.empty())
        {
            to_remove.push_back(schedule_index);
            continue;
        }

        auto lua = this->server.lua.open(entry.script, entry.func);
        if (lua)
            std::ignore = co_await lua->call(0);

        if (entry.cron.has_value() && entry.cron->empty() == false)
        {
            // Re-align from the fired slot so next time stays cron-accurate.
            auto next_time = entry.next_execution(next_execution);
            if (next_time.has_value())
                next_execution = next_time.value();
            else
                to_remove.push_back(schedule_index);
        }
        else
        {
            to_remove.push_back(schedule_index);
        }
    }

    for (auto schedule_index : to_remove)
        this->_tasks.erase(schedule_index);

    co_return;
}
