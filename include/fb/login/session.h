#ifndef __SESSION_LOGIN_H__
#define __SESSION_LOGIN_H__

#include <fb/core/socket.h>

namespace fb { namespace login {

class session
{
public:
    std::string             created_id;

public:
    session();
    ~session();
};

} }

#endif // !__SESSION_LOGIN_H__