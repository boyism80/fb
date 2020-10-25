#ifndef __SESSION_INTERNAL_H__
#define __SESSION_INTERNAL_H__

#include "module/socket/socket.h"
#include <vector>

namespace fb { namespace internal {

class session
{
public:
    std::string                 name;

public:
    session();
    ~session();
};

} }

#endif // !__SESSION_INTERNAL_H__