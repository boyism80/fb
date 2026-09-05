#include <fb/game/handler/protocol/update_move.h>
#include <fb/game/handler/protocol/move.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
update_move<V>::update_move(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::update_move<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> update_move<V>::handle(fb::socket<character>& session, game_reqs::update_move<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    auto move_handler = fb::game::handler::protocol::move<V>(this->server);
    auto moved = co_await move_handler.handle(session, request.direction, request.position, request.walk_queue_slot);
    if (moved)
        ch->update_map(*map, request.begin, request.size, request.crc);

    co_return true;
}

template class update_move<fb::protocol::CLIENT_VERSION::v550>;
template class update_move<fb::protocol::CLIENT_VERSION::v565>;
template class update_move<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
