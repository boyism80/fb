#include <fb/game/handler/timer/announce.h>

using namespace fb::game::handler::timer;
using table = fb::model::table;

announce::announce(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> announce::handle()
{
    if (table::announce.size() == 0)
        co_return;

    auto i           = random<uint16_t>(0, static_cast<uint16_t>(table::announce.size() - 1));
    auto message_str = table::announce[i].message;
    this->server.characters.foreach_enqueue([message_str](auto& ch) -> async::task<void> {
        if (ch->option(OPTION::NEWS) == false)
            co_return;

        ch->message(message_str, MESSAGE_TYPE::WORLD);
        co_return;
    });
    co_return;
}
