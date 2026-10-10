#include <fb/game/handler/timer/evaluation_timer.h>

#include <fb/game/thread_params.h>
#include <fb/logger.h>

#include <exception>
#include <thread>

using namespace fb::game::handler::timer;
using namespace fb::game;

evaluation_timer::evaluation_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> evaluation_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    co_await params->characters.foreach_async([now](auto& ch) -> async::task<void> {
        if (ch == nullptr || ch->inited() == false)
            co_return;

        if (ch->client_version != fb::protocol::CLIENT_VERSION::v651)
            co_return;

        try
        {
            ch->accumulate_evaluation_playtime(now);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("evaluation_timer error for character {}: {}", ch->id, e.what());
        }
        co_return;
    });

    co_return;
}
