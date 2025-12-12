#include <fb/game/handler/timer/marketplace_restore_timer.h>
#include <fb/game/thread_params.h>

using namespace fb::game::handler::timer;

marketplace_restore_timer::marketplace_restore_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> marketplace_restore_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& [id, character] : params->characters)
    {
        std::ignore = character->marketplace.restore();
    }

    co_return;
}

