#include <fb/game/handler/timer/schedule_timer.h>
#include <fb/game/server.h>

using namespace fb::game::handler::timer;
using table = fb::model::table;

schedule_timer::schedule_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> schedule_timer::handle()
{
    auto now = fb::model::datetime();

    for (const auto& schedule : table::schedule)
    {
        if (schedule.date.begin.has_value() && now < schedule.date.begin.value())
            continue;

        if (schedule.date.end.has_value() && now > schedule.date.end.value())
            continue;

        if (schedule.script.empty())
            continue;

        if (!schedule.repeat.has_value())
        {
            if (!schedule.date.begin.has_value() || !schedule.date.end.has_value())
                continue;

            if (schedule.date.begin.value() != schedule.date.end.value())
                continue;

            auto last_execution = this->server.schedule_last_execution(schedule.id);
            if (last_execution != fb::model::datetime())
                continue;

            if (now < schedule.date.begin.value())
                continue;

            auto lua = fb::lua::new_context();
            if (lua == nullptr)
                continue;

            try
            {
                lua->execute(schedule.script);
            }
            catch (std::exception& e)
            {
                fb::logger::warn(std::format("Schedule {} script execution failed: {}", schedule.id, e.what()));
            }

            this->server.schedule_last_execution(schedule.id, now);
            continue;
        }

        auto last_execution  = this->server.schedule_last_execution(schedule.id);
        auto repeat_interval = schedule.repeat.value();
        auto schedule_begin  = schedule.date.begin.value();

        if (last_execution == fb::model::datetime())
        {
            if (now < schedule_begin)
                continue;

            auto elapsed          = now - schedule_begin;
            auto intervals_passed = elapsed.total_milliseconds() / repeat_interval.total_milliseconds();
            auto next_execution   = schedule_begin + fb::model::timespan(std::chrono::milliseconds(
                                                       (intervals_passed + 1) * repeat_interval.total_milliseconds()));

            if (now < next_execution)
                continue;

            auto lua = fb::lua::new_context();
            if (lua == nullptr)
                continue;

            try
            {
                lua->execute(schedule.script);
            }
            catch (std::exception& e)
            {
                fb::logger::warn(std::format("Schedule {} script execution failed: {}", schedule.id, e.what()));
            }

            this->server.schedule_last_execution(schedule.id, next_execution);
            continue;
        }

        auto next_execution = last_execution + repeat_interval;
        if (now < next_execution)
            continue;

        auto lua = fb::lua::new_context();
        if (lua == nullptr)
            continue;

        try
        {
            lua->execute(schedule.script);
        }
        catch (std::exception& e)
        {
            fb::logger::warn(std::format("Schedule {} script execution failed: {}", schedule.id, e.what()));
        }

        this->server.schedule_last_execution(schedule.id, now);
    }

    co_return;
}
