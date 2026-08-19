#include <fb/login/protocol/meta_dat.h>

namespace fb::protocol::login::request {

#ifdef BOT
template <CLIENT_VERSION V>
meta_dat<V>::meta_dat(const std::string& name) :
    name(name)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void meta_dat<V>::serialize(fb::stream_writer<big_endian>& writer) const
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
void meta_dat<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto flag = reader.read<uint8_t>();
    if (flag == 1)
        this->name.clear();
    else
        this->name = reader.read<std::string, uint8_t>();
}
#endif

template class meta_dat<CLIENT_VERSION::v550>;
template class meta_dat<CLIENT_VERSION::v565>;
template class meta_dat<CLIENT_VERSION::v651>;

} // namespace fb::protocol::login::request
