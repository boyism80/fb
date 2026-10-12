#include <fb/game/handler/timer/storage_claim_restore_timer.h>

#include <fb/game/thread_params.h>

#include <thread>
#include <tuple>

using namespace fb::game::handler::timer;

storage_claim_restore_timer::storage_claim_restore_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> storage_claim_restore_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    co_await params->characters.foreach ([](auto& character) {
        std::ignore = character->storage_box.restore_claims();
    });

    co_return;
}
