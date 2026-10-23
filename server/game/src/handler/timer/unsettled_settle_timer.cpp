#include <fb/game/handler/timer/unsettled_settle_timer.h>

#include <fb/game/thread_params.h>

#include <thread>
#include <tuple>

using namespace fb::game::handler::timer;

unsettled_settle_timer::unsettled_settle_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> unsettled_settle_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    if (this->server.settle_paused)
        co_return;

    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    co_await params->characters.foreach ([](auto& character) {
        std::ignore = character->unsettled.settle();
    });

    co_return;
}
