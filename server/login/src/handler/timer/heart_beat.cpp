#include <fb/login/handler/timer/heart_beat.h>

namespace fb::login::handler::timer {

heart_beat::heart_beat(fb::login::server& server) :
    fb::handler::timer<fb::login::server>(server)
{ }

async::task<void> heart_beat::handle()
{
    co_await this->server.update_status();
    co_return;
}

} // namespace fb::login::handler::timer
