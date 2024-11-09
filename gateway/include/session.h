#ifndef __SESSION_GATEWAY_H__
#define __SESSION_GATEWAY_H__

#include <fb/socket.h>

namespace fb { namespace gateway {

/**
 * @brief      This class describes a session.
 */
class session
{
public:
    /**
     * @brief      Constructs a new instance.
     */
    session();
    /**
     * @brief      Destroys the object.
     */
    ~session();
};

}} // namespace fb::gateway

#endif // !__SESSION_GATEWAY_H__