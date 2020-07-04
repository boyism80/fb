#ifndef __GATEWAY_H__
#define __GATEWAY_H__

#include <string>
#include <vector>
#include <module/socket/socket.h>

namespace fb { namespace login {

class gateway
{
private:
    std::string             _name;
    std::string             _desc;
    uint32_t                _ip;
    uint16_t                _port;

public:
    gateway(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port);
    gateway(const std::string& name, const std::string& desc, const std::string& ip, uint16_t port);
    ~gateway();

public:
    const std::string&      name() const;
    const std::string&      dest() const;
    uint32_t                ip() const;
    uint16_t                port() const;
};

class gateways : public std::vector<fb::login::gateway*>
{
public:
    gateways();
    ~gateways();

public:
    fb::ostream             make_stream() const;
};

} }

#endif // !__GATEWAY_H__