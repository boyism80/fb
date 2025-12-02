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

    auto& gid = me->group_id();
    if (gid.has_value() && me->name() == request.name)
    {
        co_await this->server.leave_group(*me);
    }
    else
    {
        std::ignore = co_await this->server.create_group(*me, request.name);
    }
    co_return true;
}
