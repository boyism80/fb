#include <fb/game/protocol/object/miss.h>

namespace fb::protocol::game::request {

#ifdef BOT
void miss::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
}
#else
void miss::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid = reader.read<uint32_t>();
}
#endif
} // namespace fb::protocol::game::request
