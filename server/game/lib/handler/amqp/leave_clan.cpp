#include <fb/game/handler/amqp/leave_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

leave_clan::leave_clan(fb::game::server& server) :
    fb::amqp_handler<fb::game::server, internal_resp::LeaveClan>(server)
{ }

async::task<void> leave_clan::handle(const internal_resp::LeaveClan& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_clan_leave_member(message);
}
