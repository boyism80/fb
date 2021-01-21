#ifndef __SERVICE_GATEWAY_H__
#define __SERVICE_GATEWAY_H__

#include <string>
#include "protocol/gateway.h"

namespace fb { namespace gateway { namespace service { 

class gateway
{
public:
    gateway();
    ~gateway();

public:
    void                        assert_client(const fb::protocol::gateway::request::assert_version& request);
};

} } }
#endif // !__SERVICE_GATEWAY_H__
