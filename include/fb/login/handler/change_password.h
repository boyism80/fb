#ifndef __FB_LOGIN_HANDLER_CHANGE_PASSWORD_H__
#define __FB_LOGIN_HANDLER_CHANGE_PASSWORD_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler {

/**
 * @brief      Handler for password change requests.
 *
 *             Validates current credentials and updates the password.
 */
class change_password : public fb::handler<fb::login::server, fb::protocol::login::request::update_pw>
{
public:
    /**
     * @brief      Constructs a new change password handler.
     *
     * @param      server  The login server instance.
     */
    change_password(fb::login::server& server);

    // Delete copy and move constructors/assignment operators
    change_password(const change_password&)             = delete;
    change_password(change_password&&)                  = delete;
    change_password& operator= (const change_password&) = delete;
    change_password& operator= (change_password&&)      = delete;

    /**
     * @brief      Handles password change requests.
     *
     * @param      session  The client session.
     * @param      request  The password change request.
     *
     * @return     An async task returning true if handled successfully.
     */
    async::task<bool> handle(fb::socket<fb::login::session>&          session,
                             fb::protocol::login::request::update_pw& request) override;
};

} // namespace fb::login::handler

#endif // !__FB_LOGIN_HANDLER_CHANGE_PASSWORD_H__
