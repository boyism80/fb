#include <fb/game/handler/protocol/move.h>
#include <fb/game/server.h>
#include <tuple>

using namespace fb::model;

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
move<V>::move(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::move<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> move<V>::handle(fb::socket<character>&      session,
                                  DIRECTION                   direction,
                                  const fb::model::point16_t& position,
                                  uint8_t                     walk_queue_slot)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->paralysis() || ch->cover())
    {
        ch->update_position();
        co_return true;
    }

    auto       forward = ch->side_position(direction);
    const auto warp    = map->warpable(forward);
    if (warp != nullptr)
    {
        if (ch->condition(warp->condition) == false)
        {
            ch->message(_TEXT(MESSAGE_WARP_CANNOT_ACCESS));
            ch->update_position();
            co_return true;
        }

        switch (warp->dest.header)
        {
        case DSL::map:
        {
            auto params = fb::model::dsl::map(warp->dest.params);
            auto map    = this->server.maps[params.id];
            std::ignore = co_await ch->map(map, fb::model::point16_t(params.x, params.y));
        }
        break;

        case DSL::world:
        {
            auto  params      = fb::model::dsl::world(warp->dest.params);
            auto  world_table = table::world;
            auto& world       = world_table[params.id][params.index];
            ch->show_world_map(params.id, params.index);
        }
        break;

        case DSL::script:
        {
            std::ignore = ch->move(direction, position, walk_queue_slot);

            auto params = fb::model::dsl::script(warp->dest.params);
            if (params.path.empty() || params.function.empty())
                break;

            auto lua = this->server.lua.open(params.path, params.function);
            if (lua)
            {
                lua->pushobject(ch);
                std::ignore = lua->call(1);
            }
        }
        break;

        default:
            throw std::runtime_error("invalid dsl header");
        }
    }
    else
    {
        std::ignore = ch->move(direction, position, walk_queue_slot);
    }
    co_return true;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> move<V>::handle(fb::socket<character>& session, game_reqs::move<V>& request)
{
    return this->handle(session, request.direction, request.position, request.walk_queue_slot);
}

template class move<fb::protocol::CLIENT_VERSION::v550>;
template class move<fb::protocol::CLIENT_VERSION::v565>;
template class move<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
