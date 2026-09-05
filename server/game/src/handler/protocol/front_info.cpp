#include <fb/game/handler/protocol/front_info.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
front_info<V>::front_info(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::front_info<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> front_info<V>::handle(fb::socket<character>& session, game_reqs::front_info<V>& request)
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
        auto message = object->is(OBJECT_TYPE::ITEM) ? std::static_pointer_cast<fb::game::item>(object)->inven_name()
                                                     : object->name();
        ch->message(message, MESSAGE_TYPE::STATE);
    }
    co_return true;
}

template class front_info<fb::protocol::CLIENT_VERSION::v550>;
template class front_info<fb::protocol::CLIENT_VERSION::v565>;
template class front_info<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
