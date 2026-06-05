#include <fb/game/handler/amqp/kick_out.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

kick_out::kick_out(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::KickOut>(server)
{ }

async::task<void> kick_out::handle(const internal_resp::KickOut& message)
{
    auto guard = this->server.characters.enter_write();
    guard.value().on_kick_out(message);
    co_return;
}
