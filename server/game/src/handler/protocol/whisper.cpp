#include <fb/game/handler/protocol/whisper.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

whisper::whisper(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::whisper>(server)
{ }

async::task<bool> whisper::handle(fb::socket<character>& session, game_reqs::whisper& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    auto weak = me->weak_from_this_as<character>();
    auto map  = me->map();
    if (map == nullptr)
        co_return true;

    if (me->role() == ROLE::USER && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_WHISPER))
    {
        co_await me->message(_TEXT(MESSAGE_WHISPER_DISABLED_AREA));
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
            co_await ptr->message(error.value(), MESSAGE_TYPE::NOTIFY);
    }
    co_return true;
}
