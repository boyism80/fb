#ifndef __FB_LOGIN_HANDLER_LOGIN_H__
#define __FB_LOGIN_HANDLER_LOGIN_H__

#include <fb/handler.h>
#include <fb/login/server.h>
#include <string>
#include <string_view>
#include <optional>

namespace fb::login::handler::protocol {

namespace login_reqs = fb::protocol::login::request;

template <fb::protocol::CLIENT_VERSION V>
class login : public fb::handler::protocol<fb::login::server, login_reqs::login<V>>
{
public:
    login(fb::login::server& server);
    login(const login&)             = delete;
    login(login&&)                  = delete;
    login& operator= (const login&) = delete;
    login& operator= (login&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, login_reqs::login<V>& request) override;

    /// <summary>
    /// Builds a ban message string with reason and expire date.
    /// </summary>
    /// <param name="reason">Ban reason string (empty string means no reason).</param>
    /// <param name="expire_date">Optional ban expire date string.</param>
    /// <returns>Formatted ban message string.</returns>
    static std::string build_ban_message(std::string_view reason, const std::optional<std::string>& expire_date);

    /// <summary>
    /// Builds a maintenance message string with message and end time.
    /// </summary>
    /// <param name="message">Optional maintenance message string.</param>
    /// <param name="end_time">Optional maintenance end time string.</param>
    /// <returns>Formatted maintenance message string.</returns>
    static std::string build_maintenance_message(const std::optional<std::string>& message,
                                                 const std::optional<std::string>& end_time);
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_LOGIN_H__
