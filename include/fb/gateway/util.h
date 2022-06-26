#ifndef __SERVICE_GATEWAY_H__
#define __SERVICE_GATEWAY_H__

#include <string>
#include <fb/protocol/gateway.h>

namespace fb { namespace gateway { namespace util { 

void                        assert_client(const fb::protocol::gateway::request::assert_version& request);

} } }
#endif // !__SERVICE_GATEWAY_H__
