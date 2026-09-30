#ifndef __SESSION_GATEWAY_H__
#define __SESSION_GATEWAY_H__

#include <fb/protocol/client_version.h>
#include <fb/socket.h>

namespace fb { namespace gateway {

class session
{
public:
    const fb::protocol::CLIENT_VERSION client_version;

public:
    explicit session(fb::protocol::CLIENT_VERSION client_version);
    ~session();
};

}} // namespace fb::gateway

#endif // !__SESSION_GATEWAY_H__
