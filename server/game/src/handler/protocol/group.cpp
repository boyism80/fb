#include <fb/game/handler/protocol/group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

group::group(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::group>(server)
{ }

async::task<bool> group::handle(fb::socket<character>& session, game_reqs::group& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    auto weak = me->weak_from_this_as<character>();
    try
    {
        co_await this->server.groups.handle_action(*me, request.name);
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            me->message(e.what(), MESSAGE_TYPE::STATE);
    }
    co_return true;
}
