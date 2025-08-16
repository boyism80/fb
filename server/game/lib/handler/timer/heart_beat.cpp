#include <fb/game/handler/timer/heart_beat.h>

namespace fb::game::handler::timer {

heart_beat::heart_beat(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> heart_beat::handle()
{
    this->server.update_status();
    co_return;
}

} // namespace fb::game::handler::timer
