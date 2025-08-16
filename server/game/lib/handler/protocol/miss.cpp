#include <fb/game/handler/protocol/miss.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

miss::miss(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::miss>(server)
{ }

async::task<bool> miss::handle(fb::socket<character>& session, fb::protocol::game::request::miss& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    auto obj = map->objects[request.oid];
    if (obj == nullptr)
        co_return true;

    obj->update_external(*ch, false);
    fb::logger::info("Object miss for {}", obj->oid());

    co_return true;
}
