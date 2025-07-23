#include <fb/login/gateway.h>

using namespace fb::login;

gateway::gateway(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port) :
    _name(name),
    _desc(desc),
    _ip(ip),
    _port(port)
{ }

gateway::~gateway()
{ }

const std::string& gateway::name() const
{
    return this->_name;
}

const std::string& gateway::dst() const
{
    return this->_desc;
}

uint32_t gateway::ip() const
{
    return this->_ip;
}

uint16_t gateway::port() const
{
    return this->_port;
}

gateways::gateways()
{ }

gateways::~gateways()
{ }