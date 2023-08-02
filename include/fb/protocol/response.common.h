#ifndef __PROTOCOL_RESPONSE_COMMON_H__
#define __PROTOCOL_RESPONSE_COMMON_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace response {

class transfer : public fb::protocol::base::header
{
public:
#ifdef BOT
    uint32_t                ip;
    uint16_t                port;
    fb::buffer              parameter;
#else
    const uint32_t          ip;
    const uint16_t          port;
    const fb::buffer        parameter;
#endif

public:
#ifdef BOT
    transfer() : fb::protocol::base::header(0x03)
    { }
#endif
    transfer(uint32_t ip, uint16_t port, const fb::buffer& parameter) : fb::protocol::base::header(0x03),
        ip(ip), port(port), parameter(parameter)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->ip)
                  .write_u16(this->port)
                  .write_u8(this->parameter.size())
                  .write(this->parameter);
    }

#ifdef BOT
    void deserialize(fb::istream& in_stream)
    {
        this->ip = in_stream.read_u32();
        this->port = in_stream.read_u16();

        auto size = in_stream.read_u8();
        auto buffer = new uint8_t[size];
        in_stream.read(buffer, size);
        this->parameter = fb::buffer(buffer, size);
        delete[] buffer;
    }
#endif
};

} } }

#endif // !__PROTOCOL_RESPONSE_COMMON_H__