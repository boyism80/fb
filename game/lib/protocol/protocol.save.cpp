#include <fb/game/protocol/save.h>

namespace fb::protocol::game::response {

async::task<void> save::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x00);
}

} // namespace fb::protocol::game::response
