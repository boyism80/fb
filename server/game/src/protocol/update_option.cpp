#include <fb/game/protocol/update_option.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void update_option<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->option));
    if (this->option == OPTION::EXTENSION)
    {
        writer.write<bool>(this->ride);
    }
}
#else
template <CLIENT_VERSION V>
void update_option<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->option = static_cast<OPTION>(reader.read<uint8_t>());
    if (this->option == OPTION::EXTENSION)
    {
        this->ride = reader.read<bool>();
    }
}
#endif

template class update_option<CLIENT_VERSION::v550>;
template class update_option<CLIENT_VERSION::v565>;
template class update_option<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
