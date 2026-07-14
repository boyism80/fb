#include <fb/game/handler/timer/announce.h>

using namespace fb::game::handler::timer;
using table = fb::model::table;

announce::announce(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> announce::handle()
{
    if (table::announce.size() == 0)
        co_return;

    auto i = random<uint16_t>(0, static_cast<uint16_t>(table::announce.size() - 1));
    co_await this->server.characters.broadcast(table::announce[i].message, MESSAGE_TYPE::WORLD);
    co_return;
}
