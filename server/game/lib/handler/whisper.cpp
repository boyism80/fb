#include <fb/game/handler/whisper.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

whisper::whisper(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::whisper>(server)
{ }

async::task<bool> whisper::handle(fb::socket<character>& session, fb::protocol::game::request::whisper& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    auto weak = me->weak_from_this();
    auto map  = me->map();
    if (map == nullptr)
        co_return true;

    if (me->role() == ROLE::USER && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_WHISPER))
    {
        me->message("귓속말을 할 수 없는 지역입니다.");
        co_return true;
    }

    try
    {
        co_await this->server.whisper(*me, request.name, request.message);
        co_await this->server.threads.switching(weak);
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            me->message(e.what(), MESSAGE_TYPE::NOTIFY);
    }
    co_return true;
}
