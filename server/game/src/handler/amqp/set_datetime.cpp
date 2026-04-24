#include <fb/game/handler/amqp/set_datetime.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

set_datetime::set_datetime(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::SetDateTime>(server)
{ }

async::task<void> set_datetime::handle(const internal_resp::SetDateTime& message)
{
    if (message.error != 0)
        co_return;

    if (message.reset)
        this->server.reset_now_offset();
    else
        this->server.now(fb::model::datetime(message.datetime));
    co_return;
}
