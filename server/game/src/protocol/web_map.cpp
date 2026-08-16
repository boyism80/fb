#include <fb/game/protocol/web_map.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void web_map<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
template <CLIENT_VERSION V>
void web_map<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif

template class web_map<CLIENT_VERSION::v550>;
template class web_map<CLIENT_VERSION::v565>;
template class web_map<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
web_map<V>::web_map(const std::vector<web_map_entry>& entries) :
    entries(entries)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void web_map<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto count = reader.read<uint8_t>();
    this->entries.clear();
    for (uint8_t i = 0; i < count; i++)
    {
        auto entry = web_map_entry();
        entry.x    = reader.read<uint16_t>();
        entry.y    = reader.read<uint16_t>();
        entry.name = reader.read<std::string, uint8_t>();
        this->entries.push_back(entry);
    }
}
#else
template <CLIENT_VERSION V>
void web_map<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->entries.size()));
    for (const auto& entry : this->entries)
    {
        writer.write<uint16_t>(entry.x);
        writer.write<uint16_t>(entry.y);
        writer.write<std::string, uint8_t>(entry.name);
    }
}
#endif

template class web_map<CLIENT_VERSION::v550>;
template class web_map<CLIENT_VERSION::v565>;
template class web_map<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
