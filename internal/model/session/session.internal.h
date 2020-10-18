#ifndef __SESSION_INTERNAL_H__
#define __SESSION_INTERNAL_H__

#include "module/socket/socket.h"
#include <vector>

namespace fb { namespace internal {

class session
{
public:
    std::vector<std::string>    subscribed;

public:
    session();
    ~session();
};

} }

#endif // !__SESSION_INTERNAL_H__