#include <fb/login/protocol/message.h>

namespace fb::protocol::login::response {

#ifndef BOT
message::message(std::string_view text, uint8_t type) :
    text(std::string(text)),
    type(type)
{ }
#endif

#ifndef BOT
void message::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->type);
    writer.write<std::string, uint8_t>(this->text);
}
#else
void message::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = reader.read<uint8_t>();
    this->text = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::login::response
