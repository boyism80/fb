#include <fb/game/handler/timer/recovery_timer.h>

#include <fb/game/thread_params.h>
#include <fb/logger.h>

#include <exception>
#include <thread>

using namespace fb::game::handler::timer;

recovery_timer::recovery_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> recovery_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    co_await params->characters.foreach_async([now](auto& ch) -> async::task<void> {
        if (ch == nullptr)
            co_return;

        try
        {
            ch->stat.recover(now);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("recovery_timer error for character {}: {}", ch->id, e.what());
        }
        co_return;
    });

    co_return;
}
