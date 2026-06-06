#include <fb/game/protocol/freeze.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> freeze::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<bool>(this->enable);
}
#else
async::task<void> freeze::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->enable = reader.read<bool>();
}
#endif

} // namespace fb::protocol::game::response
