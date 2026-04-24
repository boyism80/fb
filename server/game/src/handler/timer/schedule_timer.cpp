#include <fb/game/handler/timer/schedule_timer.h>
#include <fb/game/server.h>

using namespace fb::game::handler::timer;
using table = fb::model::table;

schedule_timer::schedule_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> schedule_timer::handle()
{
    // Get the least loaded logic thread for Lua script execution
    auto* logic_thread = this->server.threads.least_loaded();
    if (logic_thread == nullptr)
    {
        fb::logger::warn("No logic thread available for schedule execution");
        co_return;
    }

    // Switch to logic thread context
    co_await logic_thread->switching();

    auto now       = this->server.now();
    auto to_remove = std::vector<uint32_t>{};

    for (auto& [schedule_id, next_execution] : this->server.scheduled_tasks())
    {
        if (now < next_execution)
            continue;

        // Execution time reached
        const auto& schedule = table::schedule[schedule_id];

        // Execute script (now running on logic thread)
        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
            try
            {
                lua->load(schedule.script);
                lua->func(schedule.func);
                std::ignore = co_await lua->call(0);
            }
            catch (std::exception& e)
            {
                fb::logger::warn(std::format("Schedule {} script execution failed: {}", schedule_id, e.what()));
            }
        }

        // Calculate next execution time (use repeat to decide repeating vs one-time)
        if (schedule.repeat.has_value())
        {
            // Repeating schedule
            auto next_time = next_execution + schedule.repeat.value();

            if (schedule.date.end.has_value() && next_time > schedule.date.end.value())
            {
                // End time passed - no more executions
                to_remove.push_back(schedule_id);
            }
            else
            {
                next_execution = next_time;
            }
        }
        else
        {
            // One-time schedule - no more executions
            to_remove.push_back(schedule_id);
        }
    }

    // Remove schedules that are no longer executable
    for (auto schedule_id : to_remove)
    {
        this->server.scheduled_tasks().erase(schedule_id);
    }

    co_return;
}
