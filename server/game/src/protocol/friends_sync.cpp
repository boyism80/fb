#include <fb/game/protocol/friends_sync.h>

namespace fb::protocol::game::response {

#ifndef BOT
friends_sync::friends_sync(uint8_t enabled) :
    enabled(enabled)
{ }
#endif

#ifndef BOT
void friends_sync::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->enabled);
}
#else
void friends_sync::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->enabled = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
