#include <fb/gateway/protocol/endpoint.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

using namespace fb::protocol::gateway;

endpoint::endpoint(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port) :
    name(name),
    desc(desc),
    ip(ip),
    port(port)
{ }

endpoint::endpoint(const std::string& name, const std::string& desc, const std::string& ip, uint16_t port) :
    endpoint(name, desc, inet_addr(ip.c_str()), port)
{ }

endpoint::endpoint(const endpoint& right) :
    endpoint(right.name, right.desc, right.ip, right.port)
{ }