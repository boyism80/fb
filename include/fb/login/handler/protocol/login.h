#ifndef __FB_LOGIN_HANDLER_LOGIN_H__
#define __FB_LOGIN_HANDLER_LOGIN_H__

#include <fb/handler.h>
#include <fb/login/server.h>
#include <string>
#include <optional>

namespace fb::login::handler::protocol {

class login : public fb::handler::protocol<fb::login::server, fb::protocol::login::request::login>
{
public:
    login(fb::login::server& server);
    login(const login&)             = delete;
    login(login&&)                  = delete;
    login& operator= (const login&) = delete;
    login& operator= (login&&)      = delete;

    async::task<bool> handle(fb::socket<fb::login::session>& session, fb::protocol::login::request::login& request) override;

    /// <summary>
    /// Builds a ban message string with reason and expire date.
    /// </summary>
    /// <param name="reason">Ban reason string (empty string means no reason).</param>
    /// <param name="expire_date">Optional ban expire date string.</param>
    /// <returns>Formatted ban message string.</returns>
    static std::string build_ban_message(const std::string& reason, const std::optional<std::string>& expire_date);
};

} // namespace fb::login::handler::protocol

#endif // __FB_LOGIN_HANDLER_LOGIN_H__
