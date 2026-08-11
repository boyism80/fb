#ifndef __SESSION_LOGIN_H__
#define __SESSION_LOGIN_H__

#include <fb/socket.h>
#include <fb/protocol/client_version.h>

namespace fb::login {

class session
{
public:
    uint32_t                     pk = -1;
    std::string                  name;
    fb::protocol::CLIENT_VERSION client_version = fb::protocol::CLIENT_VERSION::v550;

public:
    session();
    ~session();
};

} // namespace fb::login

#endif // __SESSION_LOGIN_H__
