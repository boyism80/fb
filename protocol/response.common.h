#ifndef __PROTOCOL_RESPONSE_COMMON_H__
#define __PROTOCOL_RESPONSE_COMMON_H__

#include <protocol/protocol.h>

namespace fb { namespace protocol { namespace response {

class transfer : public fb::protocol::base::response
{
public:
    const std::string       ip;
    const uint16_t          port;
    const fb::buffer        parameter;

public:
    transfer(const std::string& ip, uint16_t port, const fb::buffer& parameter) : 
        ip(ip), port(port), parameter(parameter)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        auto backward_size = this->parameter.size() > 0 ? 0x0B + this->parameter.size() : 0x0B;
        out_stream.write_u8(0x03)
                  .write_u32(inet_addr(this->ip.c_str()))
                  .write_u16(this->port)
                  .write_u8(this->parameter.size())
                  .write(this->parameter);
    }
};

} } }

#endif // !__PROTOCOL_RESPONSE_COMMON_H__