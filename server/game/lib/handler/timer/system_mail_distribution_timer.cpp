#include <fb/game/handler/timer/system_mail_distribution_timer.h>
#include <fb/game/server.h>
#include <algorithm>

namespace fb::game::handler::timer {

system_mail_distribution_timer::system_mail_distribution_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> system_mail_distribution_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& [id, ch] : params->characters)
    {
        if (ch == nullptr || ch->inited() == false)
            continue;

        co_await ch->process_system_mails();
    }

    co_return;
}

} // namespace fb::game::handler::timer
