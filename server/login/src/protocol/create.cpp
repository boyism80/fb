#include <fb/login/protocol/create.h>

namespace fb::protocol::login::request {

#ifndef BOT
void create::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->id = reader.read<std::string, uint8_t>();
    this->pw = reader.read<std::string, uint8_t>();
}
#else
create::create(std::string_view id, std::string_view pw) :
    id(std::string(id)),
    pw(std::string(pw))
{ }

void create::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->id);
    writer.write<std::string, uint8_t>(this->pw);
}
#endif

} // namespace fb::protocol::login::request
