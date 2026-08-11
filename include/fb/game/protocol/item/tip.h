#ifndef __PROTOCOL_GAME_TIP_H__
#define __PROTOCOL_GAME_TIP_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_tip : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x59;

public:
#ifndef BOT
    const uint16_t    position;
    const std::string message;
#else
    uint16_t    position;
    std::string message;
#endif

public:
#ifndef BOT
    item_tip(uint16_t position, std::string_view message) :
        position(position),
        message(std::string(message))
    { }
#else
    item_tip() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        header::serialize(writer);
        writer.write<uint8_t>(opcode);
        // Both client versions currently share the same tip body; v565 gates on
        // position high-byte==1 using the echoed C2S position from the client.
        writer.write<uint16_t>(this->position);
        writer.write<std::string, uint16_t>(this->message);
        writer.write<uint8_t>(0x00);
        if constexpr (V == CLIENT_VERSION::v565)
        {
            // Reserved for 5.65 list-shaped body once live capture confirms it.
            // Current layout remains the 5.50-compatible tip message format.
        }
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        header::deserialize(reader);
        this->position = reader.read<uint16_t>();
        this->message  = reader.read<std::string, uint16_t>();
        reader.read<uint8_t>(); // 0x00
    }
#endif
};

using item_tip_v550 = item_tip<CLIENT_VERSION::v550>;
using item_tip_v565 = item_tip<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response

#endif
