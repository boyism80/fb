#include <fb/game/handler/timer/announce.h>

namespace fb::game::handler::timer {

announce::announce(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> announce::handle()
{
    auto i = random<uint16_t>(0, fb::model::table::announce.size());
    co_await this->server.broadcast(fb::model::table::announce[i].message, MESSAGE_TYPE::WORLD, BROADCAST_TYPE::WORLD);
}

} // namespace fb::game::handler::timer
