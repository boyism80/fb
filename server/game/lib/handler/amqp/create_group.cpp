#include <fb/game/handler/amqp/create_group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

create_group::create_group(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::GroupDetails>(server)
{ }

async::task<void> create_group::handle(const internal_resp::GroupDetails& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    if (static_cast<internal::GroupDetailsAction>(message.action) != internal::GroupDetailsAction::Create)
        co_return;

    co_await this->server.on_create_group(message);
}
