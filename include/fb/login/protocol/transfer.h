#ifndef __PROTOCOL_TRANSFER_H__
#define __PROTOCOL_TRANSFER_H__

#include <fb/protocol/header.h>

namespace fb::protocol::response {

class transfer : public fb::protocol::header
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
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->ip);
        writer.write<uint16_t>(this->port);
        writer.write<uint8_t>(static_cast<uint8_t>(this->parameter.size()));
        writer.write(this->parameter);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->ip   = reader.read<uint32_t>();
        this->port = reader.read<uint16_t>();

        auto size   = reader.read<uint8_t>();
        auto buffer = new uint8_t[size];
        reader.read(buffer, size);
        this->parameter = fb::stream(buffer, size);
        delete[] buffer;
    }
#endif
};

} // namespace fb::protocol::response

#endif