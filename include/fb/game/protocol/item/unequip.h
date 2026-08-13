#ifndef __PROTOCOL_GAME_UNEQUIP_H__
#define __PROTOCOL_GAME_UNEQUIP_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_unequip : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x38;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const EQUIPMENT_PARTS parts;
#else
    EQUIPMENT_PARTS parts;
#endif

public:
#ifndef BOT
    item_unequip(EQUIPMENT_PARTS parts) :
        parts(parts)
    { }
#else
    item_unequip() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        header::serialize(writer);
        writer.write<uint8_t>(opcode);
        writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
        writer.write<uint8_t>(0x00);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        header::deserialize(reader);
        this->parts = static_cast<EQUIPMENT_PARTS>(reader.read<uint8_t>());
        reader.read<uint8_t>();
    }
#endif
};

#ifndef BOT
template <>
inline void item_unequip<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
}
#endif

using item_unequip_v550 = item_unequip<CLIENT_VERSION::v550>;
using item_unequip_v565 = item_unequip<CLIENT_VERSION::v565>;
using item_unequip_v651 = item_unequip<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
