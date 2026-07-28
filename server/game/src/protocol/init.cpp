#include <fb/game/protocol/init.h>

namespace fb::protocol::game::response {

#ifndef BOT
void init::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
void init::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif

} // namespace fb::protocol::game::response
