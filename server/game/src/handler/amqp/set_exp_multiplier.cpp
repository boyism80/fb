#include <fb/game/handler/amqp/set_exp_multiplier.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

set_exp_multiplier::set_exp_multiplier(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::SetExpMultiplier>(server)
{ }

async::task<void> set_exp_multiplier::handle(const internal_resp::SetExpMultiplier& message)
{
    if (message.error != 0)
        co_return;

    this->server.exp_multiplier(message.value);
    co_return;
}
