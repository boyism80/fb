#include <fb/game/handler/timer/mob_respawn_timer.h>

#include <fb/logger.h>

#include <exception>
#include <thread>

using namespace fb::game::handler::timer;

mob_respawn_timer::mob_respawn_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> mob_respawn_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    // Copy: a table reload or instance destroy may edit params->rezens while a spawn is suspended.
    auto rezens = params->rezens;
    for (auto& rezen : rezens)
    {
        try
        {
            co_await rezen->spawn(id);
        }
        catch (std::exception& e)
        {
            fb::logger::warn("rezen spawn failed (map={}): {}", rezen->map_id(), e.what());
        }
    }
    co_return;
}
