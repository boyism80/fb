#ifndef __PROTOCOL_GAME_WEB_H__
#define __PROTOCOL_GAME_WEB_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class web : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x66;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const uint8_t     type;
    const std::string address;
    const std::string message;
#else
    uint8_t     type;
    std::string address;
    std::string message;
#endif

public:
#ifndef BOT
    web(uint8_t type, std::string address, std::string message) :
        type(type),
        address(address),
        message(message)
    { }
#else
    web() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        header::serialize(writer);
        writer.write<uint8_t>(opcode);
        if constexpr (V == CLIENT_VERSION::v550)
        {
            writer.write<uint8_t>(this->type);
            writer.write<std::string, uint16_t>(this->address);
            writer.write<std::string, uint16_t>(this->message);
            writer.write<uint8_t>(0x00);
        }
        else if constexpr (V == CLIENT_VERSION::v565)
        {
            // 5.65 treats 0x66 as an item-UI subtype dispatcher.
            // Emit a minimal safe payload (subtype 3 = other) for GM /web.
            writer.write<uint8_t>(3);
            writer.write<std::string, uint16_t>(this->address);
            writer.write<std::string, uint16_t>(this->message);
            writer.write<uint8_t>(0x00);
        }
        else // CLIENT_VERSION::v651
        {
            // 6.51 same subtype layout as 5.65.
            writer.write<uint8_t>(3);
            writer.write<std::string, uint16_t>(this->address);
            writer.write<std::string, uint16_t>(this->message);
            writer.write<uint8_t>(0x00);
        }
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        header::deserialize(reader);
        this->type    = reader.read<uint8_t>();
        this->address = reader.read<std::string, uint16_t>();
        this->message = reader.read<std::string, uint16_t>();
        reader.read<uint8_t>(); // 0x00
    }
#endif
};

using web_v550 = web<CLIENT_VERSION::v550>;
using web_v565 = web<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response

#endif
