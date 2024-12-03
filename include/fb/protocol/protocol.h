#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fb/stream.h>
#include <async/task.h>

#define BIND_ID(n) static constexpr uint8_t id = (n);

namespace fb::protocol::internal {

enum class services : uint8_t
{
    GATEWAY,
    LOGIN,
    GAME
};

enum id : uint8_t
{
    SUBSCRIBE,
    TRANSFER,
    LOGIN,
    LOGOUT,
    WHISPER,
    MESSAGE,
    SHUTDOWN,
};

} // namespace fb::protocol::internal

namespace fb::protocol::base {

class header
{
protected:
    header() = default;

public:
    ~header() = default;

public:
    virtual async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_return;
    }
    virtual async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_return;
    }
};

} // namespace fb::protocol::base

namespace fb::protocol::response {

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
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
    	co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->ip);
        writer.write<uint16_t>(this->port);
        writer.write<uint8_t>(static_cast<uint8_t>(this->parameter.size()));
        writer.write(this->parameter);
    }
#else
    async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
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

#endif // !__PROTOCOL_H__
