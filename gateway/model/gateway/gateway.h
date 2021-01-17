#ifndef __GATEWAY_H__
#define __GATEWAY_H__

#include <string>
#include <vector>

namespace fb { namespace gateway {

class entry
{
private:
    std::string             _name;
    std::string             _desc;
    uint32_t                _ip;
    uint16_t                _port;

public:
    entry(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port);
    entry(const std::string& name, const std::string& desc, const std::string& ip, uint16_t port);
    entry(const entry& right);
    ~entry();

public:
    const std::string&      name() const;
    const std::string&      dst() const;
    uint32_t                ip() const;
    uint16_t                port() const;
};

} }

#endif // !__GATEWAY_H__
