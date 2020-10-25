#ifndef __PROTOCOL_RESPONSE_INTERNAL_H__
#define __PROTOCOL_RESPONSE_INTERNAL_H__

#include "protocol/protocol.h"

namespace fb { namespace protocol { namespace internal { namespace response {

enum transfer_code : uint8_t
{
    SUCCESS,
    CONNECTED,
    NOT_READY,
    UNKNOWN,
};

class transfer : public fb::protocol::base::header
{
public:
    BIND_ID(LOGIN)

public:
    std::string             name;
    transfer_code           code;
    uint16_t                map;
    uint16_t                x, y;
    std::string             ip;
    uint16_t                port;
    uint32_t                fd;

public:
    transfer() {}
    transfer(const std::string& name, transfer_code code, uint16_t map, uint16_t x, uint16_t y, const std::string& ip, uint16_t port, uint32_t fd) : 
        name(name), code(code), map(map), x(x), y(y), ip(ip), port(port), fd(fd)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(id)
                  .writestr_u8(this->name)
                  .write_u8(this->code)
                  .write_u16(this->map)
                  .write_u16(this->x)
                  .write_u16(this->y)
                  .writestr_u8(this->ip)
                  .write_u16(this->port)
                  .write_u32(this->fd);
    }

    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
        this->code = transfer_code(in_stream.read_u8());
        this->map = in_stream.read_16();
        this->x = in_stream.read_16();
        this->y = in_stream.read_16();
        this->ip = in_stream.readstr_u8();
        this->port = in_stream.read_u16();
        this->fd = in_stream.read_u32();
    }
};

} } } }

#endif // !__PROTOCOL_RESPONSE_INTERNAL_H__