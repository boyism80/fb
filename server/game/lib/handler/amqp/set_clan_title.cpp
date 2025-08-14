#include <fb/game/handler/amqp/set_clan_title.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

set_clan_title::set_clan_title(fb::game::server& server) :
    fb::amqp_handler<fb::game::server, internal_resp::SetClanTitle>(server)
{ }

async::task<void> set_clan_title::handle(const internal_resp::SetClanTitle& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.on_clan_title_changed(message);
}
