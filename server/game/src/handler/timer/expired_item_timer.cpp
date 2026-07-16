#include <fb/game/handler/timer/expired_item_timer.h>
#include <fb/game/thread_params.h>
#include <fb/logger.h>

using namespace fb::game::handler::timer;

expired_item_timer::expired_item_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> expired_item_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    co_await params->characters.foreach_async([](auto& ch) -> async::task<void> {
        if (ch == nullptr)
            co_return;

        if (ch->inited() == false)
            co_return;

        try
        {
            co_await ch->items.remove_expired();
        }
        catch (std::exception& e)
        {
            fb::logger::warn("expired_item_timer error for character {}: {}", ch->id, e.what());
        }
        co_return;
    });

    co_return;
}
