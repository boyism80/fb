#include <fb/game/handler/timer/soliloquy_timer.h>
#include <fb/game/thread_params.h>
#include <fb/game/npc.h>
#include <fb/game/object.h>
#include <fb/game/map.h>

using namespace fb::game::handler::timer;

soliloquy_timer::soliloquy_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> soliloquy_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    auto view = params->map_view;
    for (auto& map : *view)
    {
        if (map->active == false)
            continue;

        if (map->is_active() == false)
            continue;

        for (auto& [_, obj] : map->objects)
        {
            if (obj->is(OBJECT_TYPE::NPC) == false)
                continue;

            auto npc = std::static_pointer_cast<fb::game::npc>(obj);
            npc->soliloquy();
        }
    }

    co_return;
}