#include <fb/game/protocol/ad.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> ad::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint16_t>(this->url);
    writer.write<uint16_t>(this->width);
    writer.write<uint16_t>(this->height);
    writer.write<uint8_t>(this->time);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> ad::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->url    = reader.read<std::string, uint16_t>();
    this->width  = reader.read<uint16_t>();
    this->height = reader.read<uint16_t>();
    this->time   = reader.read<uint8_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
