#ifndef __FB_LOGIN_HANDLER_CREATE_ACCOUNT_H__
#define __FB_LOGIN_HANDLER_CREATE_ACCOUNT_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

/**
 * @brief      Handler for account creation requests.
 *
 *             Validates account information, reserves the username, and creates
 *             a new character with initial stats and position.
 */
class create_account : public fb::handler::protocol<fb::login::server, fb::protocol::login::request::create>
{
public:
    /**
     * @brief      Constructs a new create account handler.
     *
     * @param      server  The login server instance.
     */
    create_account(fb::login::server& server);

    // Delete copy and move constructors/assignment operators
    create_account(const create_account&)             = delete;
    create_account(create_account&&)                  = delete;
    create_account& operator= (const create_account&) = delete;
    create_account& operator= (create_account&&)      = delete;

    /**
     * @brief      Handles account creation requests.
     *
     * @param      session  The client session.
     * @param      request  The create account request.
     *
     * @return     An async task returning true if handled successfully.
     */
    async::task<bool> handle(fb::socket<fb::login::session>&       session,
                             fb::protocol::login::request::create& request) override;
};

} // namespace fb::login::handler::protocol

#endif // !__FB_LOGIN_HANDLER_CREATE_ACCOUNT_H__
