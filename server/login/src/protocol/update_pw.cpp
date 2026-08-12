#include <fb/login/protocol/update_pw.h>

namespace fb::protocol::login::request {

#ifndef BOT
template <CLIENT_VERSION V>
void update_pw<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->name     = reader.read<std::string, uint8_t>();
    this->pw       = reader.read<std::string, uint8_t>();
    this->new_pw   = reader.read<std::string, uint8_t>();
    this->birthday = reader.read<uint32_t>();
}
#else
template <CLIENT_VERSION V>
update_pw<V>::update_pw(std::string_view name, std::string_view pw, std::string_view new_pw, uint32_t birthday) :
    name(std::string(name)),
    pw(std::string(pw)),
    new_pw(std::string(new_pw)),
    birthday(birthday)
{ }

template <CLIENT_VERSION V>
void update_pw<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<std::string, uint8_t>(this->pw);
    writer.write<std::string, uint8_t>(this->new_pw);
    writer.write<uint32_t>(this->birthday);
}
#endif

template class update_pw<CLIENT_VERSION::v550>;
template class update_pw<CLIENT_VERSION::v565>;
template class update_pw<CLIENT_VERSION::v651>;

} // namespace fb::protocol::login::request
