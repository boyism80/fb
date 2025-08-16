#include <fb/game/handler/amqp/change_clan_role.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

change_clan_role::change_clan_role(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::ChangeClanRole>(server)
{ }

async::task<void> change_clan_role::handle(const internal_resp::ChangeClanRole& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_clan_change_role(message);
}
