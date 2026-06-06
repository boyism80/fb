#include <fb/login/gateway.h>

using namespace fb::login;

gateway::gateway(std::string_view name, std::string_view desc, uint32_t ip, uint16_t port) :
    _name(std::string(name)),
    _desc(std::string(desc)),
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