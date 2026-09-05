#ifndef __FB_AMQP_ROUTE_H__
#define __FB_AMQP_ROUTE_H__

#include <fb/config.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <array>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>

namespace fb {

inline fb::protocol::internal::ProcessRole process_role()
{
    auto s = fb::config<std::string_view>("role", "home");
    if (s == "cross")
        return fb::protocol::internal::ProcessRole::Cross;
    return fb::protocol::internal::ProcessRole::Home;
}

inline bool is_cross()
{
    return process_role() == fb::protocol::internal::ProcessRole::Cross;
}

inline std::string amqp_scope()
{
    if (is_cross())
        return "cross";
    return std::format("w{}", fb::config<uint32_t>("world"));
}

enum class amqp_kind
{
    unicast,
    fanout,
    home
};

struct amqp_topic
{
    const char* name;
    amqp_kind   kind;
};

inline constexpr std::array<amqp_topic, 10> k_amqp_topics{
    {
     {"game", amqp_kind::unicast},
     {"clan", amqp_kind::fanout},
     {"group", amqp_kind::fanout},
     {"mail", amqp_kind::fanout},
     {"storage", amqp_kind::fanout},
     {"ban", amqp_kind::fanout},
     {"matchmaking", amqp_kind::fanout},
     {"system", amqp_kind::fanout},
     {"castle", amqp_kind::home},
     {"global", amqp_kind::home},
     }
};

inline std::string amqp_key(std::string_view topic, std::string_view scope, uint32_t host = 0)
{
    if (topic == "game")
        return std::format("fb.game.{}.{}", scope, host);
    return std::format("fb.{}.{}", topic, scope);
}

} // namespace fb

#endif
