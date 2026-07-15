#include <fb/game/protocol/group.h>

namespace fb::protocol::game::request {

#ifdef BOT
void group::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->name);
}
#else
void group::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->name = reader.read<std::string, uint8_t>();
}
#endif
} // namespace fb::protocol::game::request
