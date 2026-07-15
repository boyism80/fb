#include <fb/game/protocol/exit.h>

namespace fb::protocol::game::request {

#ifdef BOT
void exit::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
void exit::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif
} // namespace fb::protocol::game::request
