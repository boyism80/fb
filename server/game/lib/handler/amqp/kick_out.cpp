#include <fb/game/handler/amqp/kick_out.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

kick_out::kick_out(fb::game::server& server) :
    fb::amqp_handler<fb::game::server, internal_resp::KickOut>(server)
{ }

async::task<void> kick_out::handle(const internal_resp::KickOut& message)
{
    auto ch = this->server.characters.find(message.name);
    if (ch == nullptr)
        co_return;

    auto socket = ch->socket();
    if (socket != nullptr)
        socket->close();
    co_return;
}
