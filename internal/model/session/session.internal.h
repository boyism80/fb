#ifndef __SESSION_INTERNAL_H__
#define __SESSION_INTERNAL_H__

#include "module/socket/socket.h"

namespace fb { namespace internal {

class session
{
public:
    std::string             created_id;

public:
    session();
    ~session();
};

} }

#endif // !__SESSION_INTERNAL_H__