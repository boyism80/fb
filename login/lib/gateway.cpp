#include <gateway.h>

fb::login::gateway::gateway(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port) :
    _name(name),
    _desc(desc),
    _ip(ip),
    _port(port)
{ }

fb::login::gateway::gateway(const std::string& name, const std::string& desc, const std::string& ip, uint16_t port) :
    gateway(name, desc, inet_addr(ip.c_str()), port)
{ }

fb::login::gateway::~gateway()
{ }

const std::string& fb::login::gateway::name() const
{
    return this->_name;
}

const std::string& fb::login::gateway::dst() const
{
    return this->_desc;
}

uint32_t fb::login::gateway::ip() const
{
    return this->_ip;
}

uint16_t fb::login::gateway::port() const
{
    return this->_port;
}

fb::login::gateways::gateways()
{ }

fb::login::gateways::~gateways()
{ }