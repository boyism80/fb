#ifndef __PROTOCOL_GAME_EFFECT_H__
#define __PROTOCOL_GAME_EFFECT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class effect : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x29;

public:
#ifndef BOT
    const fb::game::object& me;
    const uint8_t           value;
#else
    uint32_t oid;
    uint8_t  value;
#endif

public:
#ifndef BOT
    effect(const fb::game::object& me, uint8_t value) :
        me(me),
        value(value)
    { }
#else
    effect() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        header::serialize(writer);
        writer.write<uint8_t>(opcode);
        writer.write<uint32_t>(this->me.oid());
        writer.write<uint8_t>(this->value);
        if constexpr (V == CLIENT_VERSION::v550)
        {
            writer.write<uint8_t>(0x00);
        }
        else
        {
            writer.write<uint16_t>(0); // duration
            if (this->value != 0x86)
            {
                writer.write<uint16_t>(0); // x
                writer.write<uint16_t>(0); // y
            }
        }
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        header::deserialize(reader);
        this->oid   = reader.read<uint32_t>();
        this->value = reader.read<uint8_t>();
        if constexpr (V == CLIENT_VERSION::v550)
        {
            reader.read<uint8_t>(); // 0x00
        }
        else
        {
            reader.read<uint16_t>(); // duration
            if (this->value != 0x86)
            {
                reader.read<uint16_t>(); // x
                reader.read<uint16_t>(); // y
            }
        }
    }
#endif
};

using effect_v550 = effect<CLIENT_VERSION::v550>;
using effect_v565 = effect<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response

#endif
