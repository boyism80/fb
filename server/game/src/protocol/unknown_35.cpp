#include <fb/game/protocol/unknown_35.h>

namespace fb::protocol::game::response {

#ifndef BOT
void unknown_35::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
void unknown_35::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif

} // namespace fb::protocol::game::response
