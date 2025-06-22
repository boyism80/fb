#include <fb/game/protocol/object/miss.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> miss::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->sequence);
}
#else
async::task<void> miss::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->sequence = reader.read<uint32_t>();
}
#endif
} // namespace fb::protocol::game::request
