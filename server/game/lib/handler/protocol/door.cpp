#include <fb/game/handler/protocol/door.h>
#include <fb/game/server.h>

fb::game::handler::protocol::door::door(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::door>(server)
{ }

async::task<bool> fb::game::handler::protocol::door::handle(fb::socket<character>& session, fb::protocol::game::request::door& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        co_return true;

#if defined DEBUG | defined _DEBUG
    lua->load("scripts/interaction.lua");
#endif

    lua->func("on_door");
    lua->pushobject(ch);
    std::ignore = co_await lua->call(1);
    co_return true;
}
