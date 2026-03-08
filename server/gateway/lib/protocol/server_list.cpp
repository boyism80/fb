#include <fb/gateway/protocol/server_list.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

using namespace fb::protocol::gateway;

endpoint::endpoint(std::string_view name, std::string_view desc, uint32_t ip, uint16_t port) :
    name(std::string(name)),
    desc(std::string(desc)),
    ip(ip),
    port(port)
{ }

endpoint::endpoint(std::string_view name, std::string_view desc, std::string_view ip, uint16_t port) :
    endpoint(name, desc, inet_addr(std::string(ip).c_str()), port)
{ }

endpoint::endpoint(const endpoint& right) :
    endpoint(right.name, right.desc, right.ip, right.port)
{ }
