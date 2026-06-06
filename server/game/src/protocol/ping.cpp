#include <fb/game/protocol/ping.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> ping::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->value);
}
#else
async::task<void> ping::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->value = reader.read<uint32_t>();
}
#endif

} // namespace fb::protocol::game::response
