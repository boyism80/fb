#include <fb/game/handler/protocol/update_move.h>
#include <fb/game/handler/protocol/move.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

update_move::update_move(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::update_move>(server)
{ }

async::task<bool> update_move::handle(fb::socket<character>& session, game_reqs::update_move& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    auto move_handler = fb::game::handler::protocol::move(this->server);
    if (co_await move_handler.handle(session, request.direction, request.position))
        ch->update_map(*map, request.begin, request.size, request.crc);

    co_return true;
}
