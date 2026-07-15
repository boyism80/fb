#include <fb/game/protocol/front_info.h>

namespace fb::protocol::game::request {

#ifdef BOT
void front_info::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
void front_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif

} // namespace fb::protocol::game::request
