#include <fb/game/handler/protocol/exit.h>

#include <fb/game/server.h>

#include <cstdint>
#include <cstdlib>
#include <format>
#include <optional>
#include <string>
#include <string_view>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
exit<V>::exit(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::exit<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> exit<V>::handle(fb::socket<character>& session, game_reqs::exit<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    if (fb::config<std::optional<uint32_t>>("world").has_value())
    {
        co_await this->server.transfer(session,
                                       fb::config<std::string_view>("login:ip"),
                                       fb::config<uint16_t>("login:port"),
                                       internal::Service::Game);
    }
    else
    {
        auto ip   = fb::config<std::string>(std::format("logins:{}:ip", ch->world()), "");
        auto port = fb::config<uint16_t>(std::format("logins:{}:port", ch->world()), 0);
        if (ip.empty() || port == 0)
        {
            fb::logger::warn("Character {} exit: no login server for world {}", ch->name(), ch->world());
            session.close();
        }
        else
        {
            co_await this->server.transfer(session, ip, port, internal::Service::Game);
        }
    }
    co_return true;
}

template class exit<fb::protocol::CLIENT_VERSION::v550>;
template class exit<fb::protocol::CLIENT_VERSION::v565>;
template class exit<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
