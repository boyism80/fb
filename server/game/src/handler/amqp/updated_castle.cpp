#include <fb/game/handler/amqp/updated_castle.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

updated_castle::updated_castle(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::UpdatedCastle>(server)
{ }

async::task<void> updated_castle::handle(const internal_resp::UpdatedCastle& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.castles.apply_updated(message);
}
