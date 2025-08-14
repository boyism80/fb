#ifndef __FB_GATEWAY_HANDLER_CHECK_VERSION_H__
#define __FB_GATEWAY_HANDLER_CHECK_VERSION_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::protocol {

/**
 * @brief      Handler for client version check requests.
 *
 *             Validates client version compatibility and responds with
 *             appropriate version check results.
 */
class check_version : public fb::handler<fb::gateway::server, fb::protocol::gateway::request::version>
{
public:
    /**
     * @brief      Constructs a new check version handler.
     *
     * @param      server  The gateway server instance.
     */
    check_version(fb::gateway::server& server);

    // Delete copy and move constructors/assignment operators
    check_version(const check_version&)             = delete;
    check_version(check_version&&)                  = delete;
    check_version& operator= (const check_version&) = delete;
    check_version& operator= (check_version&&)      = delete;

    /**
     * @brief      Handles client version check requests.
     *
     * @param      session  The client session.
     * @param      request  The version check request.
     *
     * @return     An async task returning true if handled successfully.
     */
    async::task<bool> handle(fb::socket<fb::gateway::session>&        session,
                             fb::protocol::gateway::request::version& request) override;
};

} // namespace fb::gateway::handler::protocol

#endif // !__FB_GATEWAY_HANDLER_CHECK_VERSION_H__
