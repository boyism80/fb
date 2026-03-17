#include <fb/game/protocol/door.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> door::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
async::task<void> door::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}
#endif

} // namespace fb::protocol::game::request
