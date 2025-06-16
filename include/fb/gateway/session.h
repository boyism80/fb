#ifndef __SESSION_GATEWAY_H__
#define __SESSION_GATEWAY_H__

#include <fb/socket.h>

namespace fb { namespace gateway {

/**
 * @brief      Represents a client session connected to the gateway server.
 *
 *             This class stores session-specific information for clients
 *             connected to the gateway server. Gateway sessions are typically
 *             short-lived as clients only connect to get the login server list
 *             and then disconnect to connect to their chosen login server.
 */
class session
{
public:
    /**
     * @brief      Constructs a new gateway session.
     *
     *             Initializes a new session for a client connected to the gateway server.
     */
    session();

    /**
     * @brief      Destroys the gateway session.
     */
    ~session();
};

}} // namespace fb::gateway

#endif // !__SESSION_GATEWAY_H__