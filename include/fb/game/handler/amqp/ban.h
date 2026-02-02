#ifndef FB_GAME_HANDLER_AMQP_BAN_H
#define FB_GAME_HANDLER_AMQP_BAN_H

#include <fb/handler.h>
#include <fb/game/server.h>
#include <string>
#include <string_view>
#include <optional>

namespace fb::game::handler::amqp {

class ban : public fb::handler::amqp<fb::game::server, internal_resp::Ban>
{
public:
    ban(fb::game::server& server);
    ban(const ban&)             = delete;
    ban(ban&&)                  = delete;
    ban& operator= (const ban&) = delete;
    ban& operator= (ban&&)      = delete;

public:
    async::task<void> handle(const internal_resp::Ban& message) override;

    /// <summary>
    /// Builds a ban message string with reason and expire date.
    /// </summary>
    /// <param name="reason">Ban reason string (empty string means no reason).</param>
    /// <param name="expire_date">Optional ban expire date string.</param>
    /// <returns>Formatted ban message string.</returns>
    static std::string build_ban_message(std::string_view reason, const std::optional<std::string>& expire_date);
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_BAN_H
