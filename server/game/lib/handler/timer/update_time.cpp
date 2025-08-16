#include <fb/game/handler/timer/update_time.h>

namespace fb::game::handler::timer {

update_time::update_time(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> update_time::handle()
{
    this->server.update_time();
    co_return;
}

} // namespace fb::game::handler::timer
