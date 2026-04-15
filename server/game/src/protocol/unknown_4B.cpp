#include <fb/game/protocol/unknown_4B.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> unknown_4B::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint16_t>(this->payload);
}
#else
async::task<void> unknown_4B::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->payload = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
