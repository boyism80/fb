#include <fb/game/protocol/trade/bundle.h>

namespace fb::protocol::game::response {

#ifndef BOT
void trade_bundle::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(0x00);
}
#else
void trade_bundle::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 0x01
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
