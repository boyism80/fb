#include <fb/login/protocol/update_pw.h>

namespace fb::protocol::login::request {

#ifndef BOT
void update_pw::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->name     = reader.read<std::string, uint8_t>();
    this->pw       = reader.read<std::string, uint8_t>();
    this->new_pw   = reader.read<std::string, uint8_t>();
    this->birthday = reader.read<uint32_t>();
}
#else
update_pw::update_pw(std::string_view name, std::string_view pw, std::string_view new_pw, uint32_t birthday) :
    name(std::string(name)),
    pw(std::string(pw)),
    new_pw(std::string(new_pw)),
    birthday(birthday)
{ }

void update_pw::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<std::string, uint8_t>(this->pw);
    writer.write<std::string, uint8_t>(this->new_pw);
    writer.write<uint32_t>(this->birthday);
}
#endif

} // namespace fb::protocol::login::request
