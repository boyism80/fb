#include <fb/game/handler/amqp/ban.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

ban::ban(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::Ban>(server)
{ }

async::task<void> ban::handle(const internal_resp::Ban& message)
{
    auto guard = co_await this->server.characters.enter_write_async();
    co_await guard.value().on_ban(message);
}
