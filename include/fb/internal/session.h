#ifndef __SESSION_INTERNAL_H__
#define __SESSION_INTERNAL_H__

#include <fb/core/socket.h>
#include <vector>

namespace fb { namespace internal {

class session
{
public:
    std::string                         name;
    fb::protocol::internal::services    service;
    uint16_t                            group = 0;

public:
    session();
    ~session();
};

} }

#endif // !__SESSION_INTERNAL_H__