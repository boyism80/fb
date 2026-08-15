#include <fb/game/protocol/unknown_7b.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
unknown_7b<V>::unknown_7b(const std::string& name) :
    name(name)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_7b<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x00);
    writer.write<std::string, uint8_t>(this->name);
}

template <>
void unknown_7b<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    if (this->name.empty())
        writer.write<uint8_t>(1);
    else
    {
        writer.write<uint8_t>(0);
        writer.write<std::string, uint8_t>(this->name);
    }
}
#else
template <CLIENT_VERSION V>
void unknown_7b<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>();
    this->name = reader.read<std::string, uint8_t>();
}

template <>
void unknown_7b<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto flag = reader.read<uint8_t>();
    if (flag == 1)
        this->name.clear();
    else
        this->name = reader.read<std::string, uint8_t>();
}
#endif

template class unknown_7b<CLIENT_VERSION::v550>;
template class unknown_7b<CLIENT_VERSION::v565>;
template class unknown_7b<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
