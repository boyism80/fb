#include <fb/game/handler/protocol/door.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

door::door(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::door>(server)
{ }

async::task<bool> door::handle(fb::socket<character>& session, game_reqs::door& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto lua = this->server.lua.new_context();
    if (lua == nullptr)
        co_return true;

#if defined DEBUG || defined _DEBUG
    lua->load("scripts/interaction.lua");
#endif

    lua->func("on_door");
    lua->pushobject(ch);
    std::ignore = co_await lua->call(1);
    co_return true;
}
