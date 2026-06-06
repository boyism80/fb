#include <fb/game/handler/timer/mob_action_timer.h>

using namespace fb::game::handler::timer;

mob_action_timer::mob_action_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> mob_action_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& [_, map] : params->maps)
    {
        if (map->active == false)
            continue;

        if (map->is_active() == false)
            continue;

        for (auto& [_, obj] : map->objects)
        {
            if (obj->is(OBJECT_TYPE::MOB) == false)
                continue;

            auto mob = std::static_pointer_cast<fb::game::mob>(obj);
            if (mob->alive() == false)
                continue;

            if (mob->paralysis())
                continue;

            std::ignore = mob->action(now);
        }
    }
    co_return;
}