#ifndef __SESSION_LOGIN_H__
#define __SESSION_LOGIN_H__

#include <fb/socket.h>

namespace fb { namespace login {

class session
{
public:
    uint32_t                pk = -1;
    std::string             name;

public:
    session();
    ~session();
};

} }

#endif // !__SESSION_LOGIN_H__