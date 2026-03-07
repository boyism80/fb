#include <fb/game/protocol/screen_refresh_complete.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> screen_refresh_complete::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> screen_refresh_complete::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x22
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
