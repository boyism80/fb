#include <fb/game/handler/timer/matchmaking_timer.h>

#include <fb/game/thread_params.h>

#include <chrono>
#include <memory>
#include <thread>
#include <tuple>

using namespace fb::game::handler::timer;
using namespace fb::game;

matchmaking_timer::matchmaking_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> matchmaking_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    std::ignore = now;

    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();
    if (params == nullptr)
        co_return;

    auto steady_now = std::chrono::steady_clock::now();
    co_await params->characters.foreach_async([this, steady_now](auto& ch) -> async::task<void> {
        if (ch == nullptr || ch->matchmaker.reconcile_due(steady_now) == false)
            co_return;

        auto weak    = ch->template weak_from_this_as<character>();
        auto builder = this->server.threads.new_builder(weak);
        builder.func = [weak](auto&) -> async::task<void> {
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;

            co_await ptr->matchmaker.reconcile();
        };
        builder.enqueue();
        co_return;
    });
}
