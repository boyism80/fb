#ifndef __SESSION_LOGIN_H__
#define __SESSION_LOGIN_H__

#include <fb/socket.h>

namespace fb { namespace login {

/**
 * @brief      Represents a login session for a connected client.
 *
 *             This class stores session-specific information for clients
 *             connected to the login server, including their primary key
 *             and character name once authenticated.
 */
class session
{
public:
    /**
     * @brief      The primary key (user ID) of the authenticated user.
     */
    uint32_t pk = -1;

    /**
     * @brief      The character name of the authenticated user.
     */
    std::string name;

public:
    /**
     * @brief      Constructs a new login session.
     *
     *             Initializes a new session with default values.
     *             The pk is set to -1 indicating an unauthenticated session.
     */
    session();

    /**
     * @brief      Destroys the login session.
     */
    ~session();
};

}} // namespace fb::login

#endif // !__SESSION_LOGIN_H__