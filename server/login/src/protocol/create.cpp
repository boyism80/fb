#include <fb/login/protocol/create.h>

namespace fb::protocol::login::request {

#ifndef BOT
template <CLIENT_VERSION V>
void create<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->id = reader.read<std::string, uint8_t>();
    this->pw = reader.read<std::string, uint8_t>();
}

template <>
void create<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->id = reader.read<std::string, uint8_t>();
    this->pw = reader.read<std::string, uint8_t>();
    if (reader.readable_size() > 0)
        reader.read<std::string, uint8_t>();
    while (reader.readable_size() > 0)
        reader.read<uint8_t>();
}
#else
template <CLIENT_VERSION V>
create<V>::create(std::string_view id, std::string_view pw) :
    id(std::string(id)),
    pw(std::string(pw))
{ }

template <CLIENT_VERSION V>
void create<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->id);
    writer.write<std::string, uint8_t>(this->pw);
}
#endif

template class create<CLIENT_VERSION::v550>;
template class create<CLIENT_VERSION::v565>;
template class create<CLIENT_VERSION::v651>;

} // namespace fb::protocol::login::request
