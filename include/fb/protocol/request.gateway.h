#ifndef __PROTOCOL_REQUEST_GATEWAY_H__
#define __PROTOCOL_REQUEST_GATEWAY_H__

#include <fb/protocol/protocol.h>

namespace fb::protocol::gateway::request {

class assert_version : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x00;
    
public:
    uint16_t version;
    uint8_t  national_key;

public:
    assert_version() = default;
#ifdef BOT
    assert_version(uint16_t version, uint8_t national_key) :
        version(version),
        national_key(national_key)
    { }
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
    	co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint16_t>(this->version);
        writer.write<uint8_t>(this->national_key);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->version      = reader.read<uint16_t>();
        this->national_key = reader.read<uint8_t>();
    }
#endif
};

class entry_list : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x57;
    
public:
    uint8_t action;
    uint8_t index;

public:
#ifndef BOT
    entry_list() = default;
#else
    entry_list(uint8_t action, uint8_t index) :
        action(action),
        index(index)
    { }
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
    	co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->action);

        if (this->action == 0x00)
            writer.write<uint8_t>(this->index);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->action = reader.read<uint8_t>();
        if (action == 0x00)
            this->index = reader.read<uint8_t>();
    }
#endif
};

} // namespace fb::protocol::gateway::request

#endif // !__PROTOCOL_REQUEST_GATEWAY_H__