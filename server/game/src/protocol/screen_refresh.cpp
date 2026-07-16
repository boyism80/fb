#include <fb/game/protocol/screen_refresh.h>

namespace fb::protocol::game::request {

#ifdef BOT
void screen_refresh::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
void screen_refresh::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif
} // namespace fb::protocol::game::request
