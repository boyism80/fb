#include <fb/game/handler/amqp/join_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

join_clan::join_clan(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::JoinClan>(server)
{ }

async::task<void> join_clan::handle(const internal_resp::JoinClan& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_clan_join_member(message);
}
