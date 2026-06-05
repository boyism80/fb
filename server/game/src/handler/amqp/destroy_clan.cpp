#include <fb/game/handler/amqp/destroy_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

destroy_clan::destroy_clan(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::DestroyClan>(server)
{ }

async::task<void> destroy_clan::handle(const internal_resp::DestroyClan& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.clans.on_error(message.error);
    co_await this->server.clans.on_destroyed(message.clan_id, message.clan_name);
}
