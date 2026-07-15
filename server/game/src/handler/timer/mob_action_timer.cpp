#include <fb/game/handler/timer/mob_action_timer.h>
#include <fb/game/mob.h>
#include <fb/game/sector.h>
#include <fb/game/thread_params.h>

using namespace fb::game::handler::timer;

mob_action_timer::mob_action_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> mob_action_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    auto view = params->map_view;
    for (auto& map : *view)
    {
        if (map->active == false)
            continue;

        for (const sector* sector : map->sectors.actives())
        {
            auto targets = std::vector<std::shared_ptr<fb::game::mob>>();
            targets.reserve(sector->size());

            for (const auto& obj : *sector)
            {
                if (obj->is(OBJECT_TYPE::MOB) == false)
                    continue;

                targets.push_back(std::static_pointer_cast<fb::game::mob>(obj));
            }

            for (const auto& mob : targets)
            {
                if (mob->alive() == false)
                    continue;

                if (mob->paralysis())
                    continue;

                mob->action(now);
            }
        }
    }
    co_return;
}
