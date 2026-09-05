#include <fb/game/handler/protocol/group.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
group<V>::group(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::group<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> group<V>::handle(fb::socket<character>& session, game_reqs::group<V>& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    auto weak  = me->weak_from_this_as<character>();
    auto error = std::optional<std::string>{};
    try
    {
        co_await this->server.groups.handle_action(*me, request.name);
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value() && weak.expired() == false)
        me->message(error.value(), MESSAGE_TYPE::STATE);
    co_return true;
}

template class group<fb::protocol::CLIENT_VERSION::v550>;
template class group<fb::protocol::CLIENT_VERSION::v565>;
template class group<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
