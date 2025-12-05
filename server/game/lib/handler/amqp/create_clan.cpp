#include <fb/game/handler/amqp/create_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

create_clan::create_clan(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::ClanDetails>(server)
{ }

async::task<void> create_clan::handle(const internal_resp::ClanDetails& message)
{
    // If this is from HTTP response (same host), skip processing as it's already handled
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    // Only process Create action from RabbitMQ
    // Query action is for direct HTTP GET requests, not broadcasted
    if (static_cast<internal::ClanDetailsAction>(message.action) != internal::ClanDetailsAction::Create)
        co_return;

    // Create clan in memory from ClanDetails (no API call needed)
    co_await this->server.on_create_clan(message);
}

