#include <fb/game/service/schedule.h>

#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/lua.h>

#include <cstdint>
#include <format>
#include <mutex>
#include <optional>
#include <tuple>
#include <vector>

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
            this->_tasks.emplace(index, next.value());
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

    auto now = this->server.now();
    auto due = std::vector<uint32_t>{};
    {
        auto lock = std::lock_guard(this->_mutex);
        for (auto it = this->_tasks.begin(); it != this->_tasks.end();)
        {
            auto& [schedule_index, next_execution] = *it;
            if (now < next_execution)
            {
                ++it;
                continue;
            }

            if (schedule_index >= table::schedule.size())
            {
                fb::logger::warn(std::format("Schedule index {} out of range", schedule_index));
                it = this->_tasks.erase(it);
                continue;
            }

            const auto& entry = table::schedule[schedule_index];
            if (entry.script.empty() || entry.func.empty())
            {
                it = this->_tasks.erase(it);
                continue;
            }

            due.push_back(schedule_index);

            // Re-align from the fired slot so next time stays cron-accurate.
            auto next_time = std::optional<fb::model::datetime>();
            if (entry.cron.has_value() && entry.cron->empty() == false)
                next_time = entry.next_execution(next_execution);

            if (next_time.has_value())
            {
                next_execution = next_time.value();
                ++it;
            }
            else
            {
                it = this->_tasks.erase(it);
            }
        }
    }

    for (auto schedule_index : due)
    {
        const auto& entry = table::schedule[schedule_index];
        auto        lua   = this->server.lua.open(entry.script, entry.func);
        if (!lua)
            continue;

        try
        {
            std::ignore = co_await lua->call(0);
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("schedule {} {}:{} failed: {}", schedule_index, entry.script, entry.func, e.what());
        }
    }

    co_return;
}
