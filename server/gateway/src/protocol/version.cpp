#include <fb/gateway/protocol/version.h>
#include <stdexcept>

namespace fb::protocol::gateway::request {

#ifndef BOT
template <CLIENT_VERSION V>
void version<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->v      = reader.read<uint16_t>();
    this->nation = reader.read<uint8_t>();

    if (is_supported(this->v) == false)
        throw std::runtime_error("unsupported client version");
    this->client_version = static_cast<CLIENT_VERSION>(this->v);

    if constexpr (V == CLIENT_VERSION::v550)
    {
        // version + nation only
    }
    else if constexpr (V == CLIENT_VERSION::v565)
    {
        this->build = reader.read<uint16_t>();
    }
    else // CLIENT_VERSION::v651
    {
        this->build = reader.read<uint16_t>();
    }
}
#else
template <CLIENT_VERSION V>
version<V>::version(CLIENT_VERSION client_version, uint8_t nation, uint16_t build) :
    v(static_cast<uint16_t>(client_version)),
    nation(nation),
    build(build),
    client_version(client_version)
{ }

template <CLIENT_VERSION V>
void version<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->v);
    writer.write<uint8_t>(this->nation);
    if constexpr (V == CLIENT_VERSION::v550)
    {
        // version + nation only
    }
    else if constexpr (V == CLIENT_VERSION::v565)
    {
        writer.write<uint16_t>(this->build);
    }
    else // CLIENT_VERSION::v651
    {
        writer.write<uint16_t>(this->build);
    }
}
#endif

template class version<CLIENT_VERSION::v550>;
template class version<CLIENT_VERSION::v565>;
template class version<CLIENT_VERSION::v651>;

} // namespace fb::protocol::gateway::request
