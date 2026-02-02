#ifndef __GATEWAY_H__
#define __GATEWAY_H__

#include <fb/socket.h>
#include <string_view>

namespace fb::login {

class gateway
{
private:
    std::string _name;
    std::string _desc;
    uint32_t    _ip   = 0;
    uint16_t    _port = 0;

public:
    gateway(std::string_view name, std::string_view desc, uint32_t ip, uint16_t port);
    ~gateway();

public:
    const std::string& name() const;
    const std::string& dst() const;
    uint32_t           ip() const;
    uint16_t           port() const;
};

class gateways : public std::vector<std::unique_ptr<fb::login::gateway>>
{
public:
    gateways();
    ~gateways();
};

} // namespace fb::login

#endif // __GATEWAY_H__
