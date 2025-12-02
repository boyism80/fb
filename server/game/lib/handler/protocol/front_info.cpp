#include <fb/game/handler/protocol/front_info.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

front_info::front_info(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::front_info>(server)
{ }

async::task<bool> front_info::handle(fb::socket<character>& session, fb::protocol::game::request::front_info& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return false;

    auto forwards = ch->forwards();
    for (auto i = forwards.begin(); i != forwards.end(); i++)
    {
        auto object  = *i;
        auto message = object->is(OBJECT_TYPE::ITEM) ? std::static_pointer_cast<fb::game::item>(object)->inven_name() : object->name();
        ch->message(message, MESSAGE_TYPE::STATE);
    }
    co_return true;
}
