#include <fb/game/handler/protocol/whisper.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
whisper<V>::whisper(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::whisper<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> whisper<V>::handle(fb::socket<character>& session, game_reqs::whisper<V>& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    auto weak = me->weak_from_this_as<character>();
    auto map  = me->map();
    if (map == nullptr)
        co_return true;

    if (me->role() == ROLE::USER && ENUM_IN(map->model().option, MAP_OPTION::DISABLE_WHISPER))
    {
        me->message(_TEXT(MESSAGE_WHISPER_DISABLED_AREA));
        co_return true;
    }

    auto error = std::optional<std::string>{};
    try
    {
        co_await me->whisper(request.name, request.message);
        co_await this->server.threads.switching(weak);
        if (weak.lock() == nullptr)
            co_return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        auto ptr = weak.lock();
        if (ptr != nullptr)
            ptr->message(error.value(), MESSAGE_TYPE::NOTIFY);
    }
    co_return true;
}

template class whisper<fb::protocol::CLIENT_VERSION::v550>;
template class whisper<fb::protocol::CLIENT_VERSION::v565>;
template class whisper<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
