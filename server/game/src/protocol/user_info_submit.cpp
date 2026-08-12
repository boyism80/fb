#include <fb/game/protocol/user_info_submit.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void user_info_submit<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->sub);
    for (size_t i = 0; i < this->strings.size(); ++i)
    {
        writer.write<std::string, uint8_t>(this->strings[i]);
    }
}
#else
template <CLIENT_VERSION V>
void user_info_submit<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->sub = reader.read<uint8_t>();
    for (size_t i = 0; i < this->strings.size(); ++i)
    {
        this->strings[i] = reader.read<std::string, uint8_t>();
    }
}
#endif

template class user_info_submit<CLIENT_VERSION::v550>;
template class user_info_submit<CLIENT_VERSION::v565>;
template class user_info_submit<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
