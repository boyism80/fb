#include <fb/game/protocol/door.h>

namespace fb::protocol::game::request {

#ifdef BOT
void door::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
void door::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif

} // namespace fb::protocol::game::request
