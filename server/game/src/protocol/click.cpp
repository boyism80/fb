#include <fb/game/protocol/click.h>

namespace fb::protocol::game::request {

#ifdef BOT
void click::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x00); // unknown
    writer.write<uint32_t>(this->oid);
}
#else
void click::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto unknown = reader.read<uint8_t>();
    this->oid    = reader.read<uint32_t>();
}
#endif

} // namespace fb::protocol::game::request
