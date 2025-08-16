#ifndef __FB_GATEWAY_HANDLER_ENTRY_LIST_H__
#define __FB_GATEWAY_HANDLER_ENTRY_LIST_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::protocol {

/**
 * @brief      Handler for client requests for login server endpoint list.
 *
 *             Provides available login server endpoints to connecting clients
 *             for server selection and load balancing.
 */
class entry_list : public fb::handler::protocol<fb::gateway::server, fb::protocol::gateway::request::endpoint>
{
public:
    /**
     * @brief      Constructs a new entry list handler.
     *
     * @param      server  The gateway server instance.
     */
    entry_list(fb::gateway::server& server);

    // Delete copy and move constructors/assignment operators
    entry_list(const entry_list&)             = delete;
    entry_list(entry_list&&)                  = delete;
    entry_list& operator= (const entry_list&) = delete;
    entry_list& operator= (entry_list&&)      = delete;

    /**
     * @brief      Handles client requests for login server endpoint list.
     *
     * @param      session  The client session.
     * @param      request  The endpoint list request.
     *
     * @return     An async task returning true if handled successfully.
     */
    async::task<bool> handle(fb::socket<fb::gateway::session>&         session,
                             fb::protocol::gateway::request::endpoint& request) override;
};

} // namespace fb::gateway::handler::protocol

#endif // !__FB_GATEWAY_HANDLER_ENTRY_LIST_H__
