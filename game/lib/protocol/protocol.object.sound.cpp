#include <fb/game/protocol/object/sound.h>

namespace fb::protocol::game::response {

#ifndef BOT
sound::sound(const fb::game::object& me, SOUND value) :
    me(me),
    value(value)
{ }
#endif

#ifndef BOT
async::task<void> sound::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x03);
    writer.write<uint16_t>(static_cast<uint16_t>(this->value)); // sound
    writer.write<uint8_t>(100);
    writer.write<uint16_t>(0x0004);
    writer.write<uint32_t>(this->me.oid());
    writer.write<uint16_t>(0x0100);
    writer.write<uint16_t>(0x0202);
    writer.write<uint16_t>(0x0004);
    writer.write<uint16_t>(0xCCCC);
}
#else
async::task<void> sound::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x00
    reader.read<uint8_t>(); // 0x03
    this->value = static_cast<SOUND>(reader.read<uint16_t>());
    reader.read<uint8_t>();  // 100
    reader.read<uint16_t>(); // 0x0004
    this->oid = reader.read<uint32_t>();
    reader.read<uint16_t>(); // 0x0100
    reader.read<uint16_t>(); // 0x0202
    reader.read<uint16_t>(); // 0x0004
    reader.read<uint16_t>(); // 0xCCCC
}
#endif

} // namespace fb::protocol::game::response
