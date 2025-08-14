#ifndef __FB_LOGIN_HANDLER_AGREEMENT_H__
#define __FB_LOGIN_HANDLER_AGREEMENT_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::protocol {

/**
 * @brief      Handler for client agreement requests.
 *
 *             Processes encryption setup and sends the user agreement text
 *             to connecting clients during the initial handshake process.
 */
class agreement : public fb::handler<fb::login::server, fb::protocol::login::request::agreement>
{
public:
    /**
     * @brief      Constructs a new agreement handler.
     *
     * @param      server  The login server instance.
     */
    agreement(fb::login::server& server);

    // Delete copy and move constructors/assignment operators
    agreement(const agreement&)             = delete;
    agreement(agreement&&)                  = delete;
    agreement& operator= (const agreement&) = delete;
    agreement& operator= (agreement&&)      = delete;

    /**
     * @brief      Handles client agreement requests.
     *
     * @param      session  The client session.
     * @param      request  The agreement request.
     *
     * @return     An async task returning true if handled successfully.
     */
    async::task<bool> handle(fb::socket<fb::login::session>&          session,
                             fb::protocol::login::request::agreement& request) override;
};

} // namespace fb::login::handler::protocol

#endif // !__FB_LOGIN_HANDLER_AGREEMENT_H__
