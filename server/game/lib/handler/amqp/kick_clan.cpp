#include <fb/game/handler/amqp/kick_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

kick_clan::kick_clan(fb::game::server& server) :
    fb::amqp_handler<fb::game::server, internal_resp::KickClan>(server)
{ }

async::task<void> kick_clan::handle(const internal_resp::KickClan& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_clan_kick_member(message);
}
