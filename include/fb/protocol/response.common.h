#ifndef __PROTOCOL_RESPONSE_COMMON_H__
#define __PROTOCOL_RESPONSE_COMMON_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace response {

class transfer : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x03;

public:
#ifdef BOT
    uint32_t   ip;
    uint16_t   port;
    fb::stream parameter;
#else
    const uint32_t   ip;
    const uint16_t   port;
    const fb::stream parameter;
#endif

public:
#ifdef BOT
    transfer() = default;
#else
    transfer(uint32_t ip, uint16_t port, const fb::stream& parameter) :
        ip(ip),
        port(port),
        parameter(parameter)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->ip)
            .write<uint16_t>(this->port)
            .write<uint8_t>(static_cast<uint8_t>(this->parameter.size()))
            .write(this->parameter);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->ip    = reader.read<uint32_t>();
        this->port  = reader.read<uint16_t>();

        auto size   = reader.read<uint8_t>();
        auto buffer = new uint8_t[size];
        reader.read(buffer, size);
        this->parameter = fb::stream(buffer, size);
        delete[] buffer;
    }
#endif
};

}}} // namespace fb::protocol::response

#endif // !__PROTOCOL_RESPONSE_COMMON_H__