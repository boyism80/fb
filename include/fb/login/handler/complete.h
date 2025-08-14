#ifndef __FB_LOGIN_HANDLER_COMPLETE_H__
#define __FB_LOGIN_HANDLER_COMPLETE_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler {

/**
 * @brief      Handler for login completion requests.
 *
 *             Finalizes the login process and prepares for character selection.
 */
class complete : public fb::handler<fb::login::server, fb::protocol::login::request::complete>
{
public:
    /**
     * @brief      Constructs a new complete handler.
     *
     * @param      server  The login server instance.
     */
    complete(fb::login::server& server);

    // Delete copy and move constructors/assignment operators
    complete(const complete&)             = delete;
    complete(complete&&)                  = delete;
    complete& operator= (const complete&) = delete;
    complete& operator= (complete&&)      = delete;

    /**
     * @brief      Handles login completion requests.
     *
     * @param      session  The client session.
     * @param      request  The completion request.
     *
     * @return     An async task returning true if handled successfully.
     */
    async::task<bool> handle(fb::socket<fb::login::session>&         session,
                             fb::protocol::login::request::complete& request) override;
};

} // namespace fb::login::handler

#endif // !__FB_LOGIN_HANDLER_COMPLETE_H__
