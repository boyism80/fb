#include <fb/game/handler/timer/storage_pending_distribution_timer.h>

using namespace fb::game::handler::timer;

storage_pending_distribution_timer::storage_pending_distribution_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> storage_pending_distribution_timer::handle(const fb::model::datetime&, std::thread::id id)
{
    this->server.poll.storage_pending.read([this, id](const std::vector<fb::game::storage_box::pending_box>& pending) {
        if (pending.empty())
            return;

        auto thread = this->server.threads.at(id);
        auto params = thread->template data<thread_params>();

        for (auto& [uid, ch] : params->characters)
        {
            if (ch == nullptr || ch->inited() == false)
                continue;

            ch->storage_box.apply_pending(pending);
        }
    });

    co_return;
}
