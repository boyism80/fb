#include <fb/game/handler/timer/mob_respawn_timer.h>

using namespace fb::game::handler::timer;

mob_respawn_timer::mob_respawn_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> mob_respawn_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& rezen : params->rezens)
    {
        std::ignore = rezen.spawn(id);
    }
    co_return;
}
