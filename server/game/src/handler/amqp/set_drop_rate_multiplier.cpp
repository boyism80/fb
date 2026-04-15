#include <fb/game/handler/amqp/set_drop_rate_multiplier.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

set_drop_rate_multiplier::set_drop_rate_multiplier(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::SetDropRateMultiplier>(server)
{ }

async::task<void> set_drop_rate_multiplier::handle(const internal_resp::SetDropRateMultiplier& message)
{
    if (message.error != 0)
        co_return;

    this->server.drop_rate_multiplier(message.value);
    co_return;
}
