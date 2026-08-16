#include <fb/game/handler/protocol/browser.h>
#include <fb/game/protocol/browser.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> browser<V>::handle(fb::socket<character>& session, game_reqs::browser<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    fb::logger::info("browser type={} from {}", request.type, ch->name());

    if constexpr (game_resp::browser<V>::supported)
    {
        static constexpr auto kUrl    = "http://example.com/";
        static constexpr auto kCookie = "uid=1&boardinfo=test";
        std::ignore                   = ch->send(game_resp::browser<V>(0, kUrl, kUrl, kCookie));
    }
    co_return true;
}

template class browser<fb::protocol::CLIENT_VERSION::v550>;
template class browser<fb::protocol::CLIENT_VERSION::v565>;
template class browser<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
