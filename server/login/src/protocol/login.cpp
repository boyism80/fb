#include <fb/login/protocol/login.h>

namespace fb::protocol::login::request {

#ifndef BOT
template <CLIENT_VERSION V>
void login<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->id = reader.read<std::string, uint8_t>();
    this->pw = reader.read<std::string, uint8_t>();
}
#else
template <CLIENT_VERSION V>
void login<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->id);
    writer.write<std::string, uint8_t>(this->pw);
}
#endif

template class login<CLIENT_VERSION::v550>;
template class login<CLIENT_VERSION::v565>;
template class login<CLIENT_VERSION::v651>;

} // namespace fb::protocol::login::request
