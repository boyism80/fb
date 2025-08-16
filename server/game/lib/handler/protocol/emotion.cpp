#include <fb/game/handler/protocol/emotion.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

emotion::emotion(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::emotion>(server)
{ }

async::task<bool> emotion::handle(fb::socket<character>& session, fb::protocol::game::request::emotion& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (request.value <= 13 || request.value == 0xFE || request.value == 0xFF)
        ch->action(ACTION(static_cast<int>(ACTION::EMOTION) + request.value), DURATION::EMOTION);
    co_return true;
}
