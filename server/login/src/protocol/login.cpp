#include <fb/login/protocol/login.h>

namespace fb::protocol::login::request {

#ifndef BOT
void login::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->id = reader.read<std::string, uint8_t>();
    this->pw = reader.read<std::string, uint8_t>();
}
#else
void login::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->id);
    writer.write<std::string, uint8_t>(this->pw);
}
#endif

} // namespace fb::protocol::login::request
