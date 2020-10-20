#ifndef __PROTOCOL_RESPONSE_INTERNAL_H__
#define __PROTOCOL_RESPONSE_INTERNAL_H__

#include "protocol/protocol.h"

namespace fb { namespace protocol { namespace internal { namespace response {

class login : public fb::protocol::base::header
{
public:
    BIND_ID(LOGIN)

public:
    std::string             name;
    bool                    status;
    std::string             ip;
    uint16_t                port;
    uint32_t                fd;

public:
    login() {}
    login(const std::string& name, bool status, const std::string& ip, uint16_t port, uint32_t fd) : 
        name(name), status(status), ip(ip), port(port), fd(fd)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name)
                  .write_u8(this->status)
                  .writestr_u8(this->ip)
                  .write_u16(this->port)
                  .write_u32(this->fd);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
        this->status = in_stream.read_u8();
        this->ip = in_stream.readstr_u8();
        this->port = in_stream.read_u16();
        this->fd = in_stream.read_u32();
    }
};

} } } }

#endif // !__PROTOCOL_RESPONSE_INTERNAL_H__