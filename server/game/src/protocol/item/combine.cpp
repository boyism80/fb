#include <fb/game/protocol/item/combine.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void item_combine<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->indices.size()));
    for (const auto& index : this->indices)
    {
        writer.write<uint8_t>(index + 1);
    }
}
#else
template <CLIENT_VERSION V>
void item_combine<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto count = reader.read<uint8_t>();
    for (int i = 0; i < count; i++)
    {
        this->indices.push_back(reader.read<uint8_t>() - 1);
    }
}
#endif

template class item_combine<CLIENT_VERSION::v550>;
template class item_combine<CLIENT_VERSION::v565>;
template class item_combine<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
