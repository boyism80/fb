#include <fb/game/protocol/init.h>

namespace fb::protocol::game::response {

async::task<void> init::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x06);
    writer.write<uint8_t>(0x00);
}
#ifdef BOT
async::task<void> init::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>();
    reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
