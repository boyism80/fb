#include <fb/game/protocol/unknown_4B.h>

namespace fb::protocol::game::response {

#ifndef BOT
void unknown_4B::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint16_t>(this->payload);
}
#else
void unknown_4B::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->payload = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
