#ifndef __FB_LOGIN_HANDLER_LOGIN_H__
#define __FB_LOGIN_HANDLER_LOGIN_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler {

/**
 * @brief      Handler for user login requests.
 *
 *             Processes user authentication, validates credentials, and
 *             transfers clients to appropriate game servers upon successful login.
 */
class login : public fb::handler<fb::login::server, fb::protocol::login::request::login>
{
public:
    /**
     * @brief      Constructs a new login handler.
     *
     * @param      server  The login server instance.
     */
    login(fb::login::server& server);

    // Delete copy and move constructors/assignment operators
    login(const login&)             = delete;
    login(login&&)                  = delete;
    login& operator= (const login&) = delete;
    login& operator= (login&&)      = delete;

    /**
     * @brief      Handles user login requests.
     *
     * @param      session  The client session.
     * @param      request  The login request.
     *
     * @return     An async task returning true if handled successfully.
     */
    async::task<bool> handle(fb::socket<fb::login::session>&      session,
                             fb::protocol::login::request::login& request) override;
};

} // namespace fb::login::handler

#endif // !__FB_LOGIN_HANDLER_LOGIN_H__
