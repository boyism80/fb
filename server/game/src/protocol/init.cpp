#include <fb/game/protocol/init.h>

namespace fb::protocol::game::response {

#ifndef BOT
void init::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x06);
    writer.write<uint8_t>(0x00);
}
#else
void init::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>();
    reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
