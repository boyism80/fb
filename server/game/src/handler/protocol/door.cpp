#include <fb/game/handler/protocol/door.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
door<V>::door(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::door<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> door<V>::handle(fb::socket<character>& session, game_reqs::door<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto lua = this->server.lua.open("scripts/interaction.lua", "on_door");
    if (!lua)
        co_return true;

    lua->pushobject(ch);
    std::ignore = co_await lua->call(1);
    co_return true;
}

template class door<fb::protocol::CLIENT_VERSION::v550>;
template class door<fb::protocol::CLIENT_VERSION::v565>;
template class door<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
