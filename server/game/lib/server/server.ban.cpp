#include <fb/game/server.h>

using namespace fb::game;

async::task<internal_resp::Ban> server::ban(const std::string&             name,
                                            const std::string&             reason,
                                            const std::optional<uint32_t>& days)
{
    auto section = fb::config<std::string>("section");
    auto&& resp = co_await this->http.post("internal", "/ban/add", internal_reqs::Ban{section, name, reason, days});
    co_return std::move(resp);
}

async::task<internal_resp::Unban> server::unban(const std::string& name)
{
    auto section = fb::config<std::string>("section");
    auto&& resp = co_await this->http.post("internal", "/ban/remove", internal_reqs::Unban{section, name});
    co_return std::move(resp);
}
