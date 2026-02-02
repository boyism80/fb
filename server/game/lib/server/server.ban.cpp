#include <fb/game/server.h>

using namespace fb::game;

async::task<internal_resp::Ban> server::ban(std::string_view               name,
                                            std::string_view               reason,
                                            const std::optional<uint32_t>& days)
{
    auto   name_str   = std::string(name);
    auto   reason_str = std::string(reason);
    auto   world      = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->http.post("internal", "/ban/add", internal_reqs::Ban{world, name_str, reason_str, days});
    co_return std::move(resp);
}

async::task<internal_resp::Unban> server::unban(std::string_view name)
{
    auto   name_str = std::string(name);
    auto   world    = fb::config<uint32_t>("world");
    auto&& resp     = co_await this->http.post("internal", "/ban/remove", internal_reqs::Unban{world, name_str});
    co_return std::move(resp);
}
