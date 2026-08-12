#ifndef __SESSION_LOGIN_H__
#define __SESSION_LOGIN_H__

#include <fb/socket.h>
#include <fb/protocol/client_version.h>

namespace fb::login {

class session
{
public:
    uint32_t                           pk = -1;
    std::string                        name;
    const fb::protocol::CLIENT_VERSION client_version;
    const fb::protocol::CLIENT_UI_MODE ui_mode;

public:
    session(fb::protocol::CLIENT_VERSION client_version,
            fb::protocol::CLIENT_UI_MODE ui_mode = fb::protocol::CLIENT_UI_MODE::OLD);
    ~session();
};

} // namespace fb::login

#endif // __SESSION_LOGIN_H__
