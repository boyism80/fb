#include <fb/game/handler/amqp/updated_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

updated_clan::updated_clan(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::UpdatedClan>(server)
{ }

async::task<void> updated_clan::handle(const internal_resp::UpdatedClan& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_updated_clan(message);
}
