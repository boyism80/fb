#include <fb/game/handler/amqp/set_datetime.h>
#include <fb/game/server.h>
#include <fb/game/inventory.h>

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

    this->server.characters.foreach_enqueue([](auto& ch) -> async::task<void> {
        for (auto i = 0; i < CONTAINER_CAPACITY; ++i)
        {
            auto spell = ch->spells[i];
            if (spell != nullptr)
                spell->delay(0);
        }
        co_return;
    });
    co_return;
}
