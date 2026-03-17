#include <fb/game/protocol/bright.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> bright::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(std::max(0, 20 - this->value));
}
#else
async::task<void> bright::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x00
    this->value = reader.read<uint8_t>();
}
#endif
} // namespace fb::protocol::game::response
