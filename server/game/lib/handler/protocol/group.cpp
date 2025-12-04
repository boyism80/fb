#include <fb/game/handler/protocol/group.h>
#include <fb/game/server.h>

fb::game::handler::protocol::group::group(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::group>(server)
{ }

async::task<bool> fb::game::handler::protocol::group::handle(fb::socket<character>& session, fb::protocol::game::request::group& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    auto weak = me->weak_from_this_as<character>();
    try
    {
        co_await this->server.handle_group_action(*me, request.name);
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            me->message(e.what(), MESSAGE_TYPE::STATE);
    }
    co_return true;
}
