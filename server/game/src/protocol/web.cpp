#include <fb/game/protocol/web.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> web::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->type);
    writer.write<std::string, uint16_t>(this->address);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> web::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->type    = reader.read<uint8_t>();
    this->address = reader.read<std::string, uint16_t>();
    this->message = reader.read<std::string, uint16_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
