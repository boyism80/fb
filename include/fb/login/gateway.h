#ifndef __GATEWAY_H__
#define __GATEWAY_H__

#include <fb/socket.h>

namespace fb { namespace login {

/**
 * @brief      Represents a game server gateway with connection information.
 *
 *             This class stores information about a game server gateway including
 *             its name, description, IP address, and port number. It's used to
 *             provide clients with server selection options during login.
 */
class gateway
{
private:
    std::string _name;
    std::string _desc;
    uint32_t    _ip   = 0;
    uint16_t    _port = 0;

public:
    /**
     * @brief      Constructs a new gateway instance.
     *
     * @param[in]  name  The name of the gateway server.
     * @param[in]  desc  The description of the gateway server.
     * @param[in]  ip    The IP address of the gateway server.
     * @param[in]  port  The port number of the gateway server.
     */
    gateway(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port);

    /**
     * @brief      Destroys the gateway object.
     */
    ~gateway();

public:
    /**
     * @brief      Gets the name of the gateway server.
     *
     * @return     The name of the gateway server.
     */
    const std::string& name() const;

    /**
     * @brief      Gets the description of the gateway server.
     *
     * @return     The description of the gateway server.
     */
    const std::string& dst() const;

    /**
     * @brief      Gets the IP address of the gateway server.
     *
     * @return     The IP address as a 32-bit integer.
     */
    uint32_t ip() const;

    /**
     * @brief      Gets the port number of the gateway server.
     *
     * @return     The port number as a 16-bit integer.
     */
    uint16_t port() const;
};

/**
 * @brief      A collection of gateway servers.
 *
 *             This class extends std::vector to provide a container for managing
 *             multiple gateway server instances. Each gateway is stored as a
 *             unique_ptr for automatic memory management.
 */
class gateways : public std::vector<std::unique_ptr<fb::login::gateway>>
{
public:
    /**
     * @brief      Constructs a new gateways collection.
     */
    gateways();

    /**
     * @brief      Destroys the gateways collection.
     */
    ~gateways();
};

}} // namespace fb::login

#endif // !__GATEWAY_H__
