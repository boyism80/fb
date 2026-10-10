#include <fb/game/handler/protocol/reputation.h>

#include <fb/game/protocol/reputation.h>
#include <fb/game/server.h>

#include <exception>
#include <optional>
#include <string>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> reputation<V>::handle(fb::socket<character>& session, game_reqs::reputation<V>& request)
{
    auto me = session.data();
    if (me == nullptr || me->inited() == false)
        co_return true;

    if (request.subtype != 0x00)
        co_return true;

    auto weak  = me->weak_from_this_as<character>();
    auto error = std::optional<std::string>{};
    try
    {
        co_await me->evaluate(request.name, request.raise);
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        auto ptr = weak.lock();
        if (ptr != nullptr)
            ptr->message(error.value(), MESSAGE_TYPE::STATE);
    }
    co_return true;
}

template class reputation<fb::protocol::CLIENT_VERSION::v550>;
template class reputation<fb::protocol::CLIENT_VERSION::v565>;
template class reputation<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
