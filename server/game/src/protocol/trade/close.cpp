#include <fb/game/protocol/trade/close.h>

namespace fb::protocol::game::response {

#ifndef BOT
trade_close::trade_close(std::string_view message) :
    message(std::string(message))
{ }
#endif

#ifndef BOT
void trade_close::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x04);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}
#else
void trade_close::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 0x04
    this->message = reader.read<std::string, uint16_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
