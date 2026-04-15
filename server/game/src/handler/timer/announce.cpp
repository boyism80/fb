#include <fb/game/handler/timer/announce.h>

using namespace fb::game::handler::timer;
using table = fb::model::table;

announce::announce(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> announce::handle()
{
    auto i = random<uint16_t>(0, table::announce.size());
    co_await this->server.broadcast(table::announce[i].message, MESSAGE_TYPE::WORLD, BROADCAST_TYPE::WORLD);
}
