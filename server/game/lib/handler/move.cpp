#include <fb/game/handler/move.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

move::move(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::move>(server)
{ }

async::task<bool> move::handle(fb::socket<character>&      session,
                               DIRECTION                   direction,
                               const fb::model::point16_t& position)
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

    // TODO: 실제로 이동하지 않고 이동했을때의 위치를 구해서
    // 해당 위치에서 워프가 가능한지 확인하고
    // 워프가능하면 워프처리, 그렇지 않다면 해당 위치로 이동한다.
    auto forward = ch->side_position(direction);

    // 워프 위치라면 워프한다.
    const auto warp = map->warpable(forward);
    if (warp != nullptr)
    {
        if (ch->condition(warp->condition) == false)
        {
            ch->message("감히 접근할 수 없습니다.");
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
            auto  params = fb::model::dsl::world(warp->dest.params);
            auto& world  = this->server.model.world[params.id][params.index];
            ch->show_world_map(params.id, params.index);
        }
        break;

        case DSL::script:
        {
            ch->move(direction, position);

            auto params = fb::model::dsl::script(warp->dest.params);
            auto lua    = fb::lua::new_context();
            if (lua != nullptr)
            {
#if defined DEBUG | defined _DEBUG
                lua->load(params.path);
#endif
                lua->func(params.function);
                lua->pushobject(ch);
                std::ignore = lua->call(1, false);
            }
        }
        break;

        default:
            throw std::runtime_error("invalid dsl header");
        }
    }
    else
    {
        ch->move(direction, position);
    }
    co_return true;
}

async::task<bool> move::handle(fb::socket<character>& session, fb::protocol::game::request::move& request)
{
    return this->handle(session, request.direction, request.position);
}