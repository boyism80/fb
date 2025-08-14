#include <fb/game/handler/amqp/leave_group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

leave_group::leave_group(fb::game::server& server) :
    fb::amqp_handler<fb::game::server, internal_resp::LeaveGroup>(server)
{ }

async::task<void> leave_group::handle(const internal_resp::LeaveGroup& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_leave_group(message);
}
