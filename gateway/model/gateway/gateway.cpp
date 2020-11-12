#include "gateway.h"
#include "module/socket/socket.h" // 헤더에 박으면 WinSock.h has already been included

fb::gateway::entry::entry(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port) : 
    _name(name), _desc(desc), _ip(ip), _port(port)
{ }

fb::gateway::entry::entry(const std::string & name, const std::string & desc, const std::string & ip, uint16_t port) : 
    entry(name, desc, inet_addr(ip.c_str()), port)
{ }

fb::gateway::entry::entry(const entry & right) : 
    entry(right._name, right._desc, right._ip, right._port)
{
}

fb::gateway::entry::~entry()
{ }

const std::string& fb::gateway::entry::name() const
{
    return this->_name;
}

const std::string& fb::gateway::entry::dest() const
{
    return this->_desc;
}

uint32_t fb::gateway::entry::ip() const
{
    return this->_ip;
}

uint16_t fb::gateway::entry::port() const
{
    return this->_port;
}