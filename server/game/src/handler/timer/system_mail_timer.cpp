#include <fb/game/handler/timer/system_mail_timer.h>

using namespace fb::game::handler::timer;

system_mail_timer::system_mail_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> system_mail_timer::handle()
{
    co_await this->server.system_mail.poll_and_deliver();
}
