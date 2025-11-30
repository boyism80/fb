#include <fb/gateway/handler/timer/heart_beat.h>

namespace fb::gateway::handler::timer {

heart_beat::heart_beat(fb::gateway::server& server) :
    fb::handler::timer<fb::gateway::server>(server)
{ }

async::task<void> heart_beat::handle()
{
    co_await this->server.update_status();
    co_return;
}

} // namespace fb::gateway::handler::timer

