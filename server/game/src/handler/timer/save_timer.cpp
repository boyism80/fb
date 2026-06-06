#include <fb/game/handler/timer/save_timer.h>
#include <fb/game/thread_params.h>

using namespace fb::game::handler::timer;

save_timer::save_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> save_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& [id, character] : params->characters)
    {
        co_await this->server.save(*character);
    }

    co_return;
}