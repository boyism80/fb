#include <fb/game/protocol/post.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> post::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->slot);
}
#else
async::task<void> post::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->slot = reader.read<uint8_t>();
}
#endif
} // namespace fb::protocol::game::request
