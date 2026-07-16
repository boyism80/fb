#include <fb/game/protocol/attack.h>

namespace fb::protocol::game::request {

#ifndef BOT
void attack::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#else
void attack::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#endif

} // namespace fb::protocol::game::request
