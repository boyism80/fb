#ifndef __SESSION_LOGIN_H__
#define __SESSION_LOGIN_H__

#include <fb/socket.h>

namespace fb { namespace login {

/**
 * @brief      This class describes a session.
 */
class session
{
public:
    uint32_t    pk = -1;
    std::string name;

public:
    /**
     * @brief      Constructs a new instance.
     */
    session();
    /**
     * @brief      Destroys the object.
     */
    ~session();
};

}} // namespace fb::login

#endif // !__SESSION_LOGIN_H__