#include <fb/game/handler/protocol/web_map.h>
#include <fb/game/protocol/web_map.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> web_map<V>::handle(fb::socket<character>& session, game_reqs::web_map<V>&)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    if constexpr (game_resp::web_map<V>::supported)
    {
        auto entries = std::vector<game_resp::web_map_entry>{};
        auto map     = ch->map();
        if (map != nullptr)
        {
            map->objects.foreach (OBJECT_TYPE::CHARACTER, [&](object& obj) {
                auto& other = static_cast<character&>(obj);
                if (&other == ch)
                    return true;
                if (other.super_hide())
                    return true;
                if (entries.size() >= 255)
                    return false;

                auto position = other.position();
                auto entry    = game_resp::web_map_entry();
                entry.x       = position.x;
                entry.y       = position.y;
                entry.name    = other.name();
                entries.push_back(std::move(entry));
                return true;
            });
        }

        std::ignore = ch->send(game_resp::web_map<V>(entries));
    }
    co_return true;
}

template class web_map<fb::protocol::CLIENT_VERSION::v550>;
template class web_map<fb::protocol::CLIENT_VERSION::v565>;
template class web_map<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
