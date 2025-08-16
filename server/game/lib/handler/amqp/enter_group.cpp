#include <fb/game/handler/amqp/enter_group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

enter_group::enter_group(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::EnterGroup>(server)
{ }

async::task<void> enter_group::handle(const internal_resp::EnterGroup& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_enter_group(message);
}
