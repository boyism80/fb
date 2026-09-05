#include <fb/game/protocol/bright.h>

namespace fb::protocol::game::response {

#ifndef BOT
bright::bright(uint8_t value) :
    value(value)
{ }
#endif

#ifndef BOT
void bright::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(std::max(0, 20 - this->value));
}
#else
void bright::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 0x00
    this->value = reader.read<uint8_t>();
}
#endif
} // namespace fb::protocol::game::response
