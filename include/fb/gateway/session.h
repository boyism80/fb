#ifndef __SESSION_GATEWAY_H__
#define __SESSION_GATEWAY_H__

#include <fb/socket.h>
#include <fb/protocol/client_version.h>

namespace fb { namespace gateway {

class session
{
public:
    fb::protocol::CLIENT_VERSION client_version = fb::protocol::CLIENT_VERSION::v550;

public:
    session();
    ~session();
};

}} // namespace fb::gateway

#endif // !__SESSION_GATEWAY_H__
