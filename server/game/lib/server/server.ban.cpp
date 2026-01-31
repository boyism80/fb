#include <fb/game/server.h>

using namespace fb::game;

async::task<internal_resp::Ban> server::ban(const std::string&             name,
                                            const std::string&             reason,
                                            const std::optional<uint32_t>& days)
{
    auto world = fb::config<uint32_t>("world");
    auto&& resp = co_await this->http.post("internal", "/ban/add", internal_reqs::Ban{world, name, reason, days});
    co_return std::move(resp);
}

async::task<internal_resp::Unban> server::unban(const std::string& name)
{
    auto world = fb::config<uint32_t>("world");
    auto&& resp = co_await this->http.post("internal", "/ban/remove", internal_reqs::Unban{world, name});
    co_return std::move(resp);
}
