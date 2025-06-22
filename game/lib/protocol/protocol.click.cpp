#include <fb/game/protocol/click.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> click::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00); // unknown
    writer.write<uint32_t>(this->fd);
}
#else
[[nodiscard]] async::task<void> click::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    auto unknown = reader.read<uint8_t>();
    this->fd     = reader.read<uint32_t>();
}
#endif

} // namespace fb::protocol::game::request
