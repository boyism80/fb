#include <fb/game/protocol/message.h>

namespace fb::protocol::game::response {

#ifndef BOT
message::message(std::string_view text, MESSAGE_TYPE type) :
    text(std::string(text)),
    type(type)
{ }
#endif

#ifndef BOT
void message::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<std::string, uint16_t>(this->text);
}
#else
void message::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = (MESSAGE_TYPE)reader.read<uint8_t>();
    this->text = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response