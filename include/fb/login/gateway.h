#ifndef __GATEWAY_H__
#define __GATEWAY_H__

#include <fb/socket.h>

namespace fb { namespace login {

/**
 * @brief      This class describes a gateway.
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
     * @brief      Constructs a new instance.
     *
     * @param[in]  name  The name
     * @param[in]  desc  The description
     * @param[in]  ip    { parameter_description }
     * @param[in]  port  The port
     */
    gateway(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port);
    /**
     * @brief      Destroys the object.
     */
    ~gateway();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::string& name() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::string& dst() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t ip() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t port() const;
};

/**
 * @brief      This class describes gateways.
 */
class gateways : public std::vector<std::unique_ptr<fb::login::gateway>>
{
public:
    /**
     * @brief      Constructs a new instance.
     */
    gateways();
    /**
     * @brief      Destroys the object.
     */
    ~gateways();
};

}} // namespace fb::login

#endif // !__GATEWAY_H__
