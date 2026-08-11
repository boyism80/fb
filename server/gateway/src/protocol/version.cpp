#include <fb/gateway/protocol/version.h>
#include <stdexcept>

namespace fb::protocol::gateway::request {

#ifndef BOT
void version::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->v      = reader.read<uint16_t>();
    this->nation = reader.read<uint8_t>();

    if (try_parse(this->v, this->client_version) == false)
        throw std::runtime_error("unsupported client version");

    if (this->client_version == CLIENT_VERSION::v565)
    {
        if (reader.readable_size() >= sizeof(uint16_t))
            this->build = reader.read<uint16_t>();
        else
            this->build = 0;
    }
}
#else
version::version(CLIENT_VERSION client_version, uint8_t nation, uint16_t build) :
    v(static_cast<uint16_t>(client_version)),
    nation(nation),
    build(build),
    client_version(client_version)
{ }

void version::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->v);
    writer.write<uint8_t>(this->nation);
    if (this->client_version == CLIENT_VERSION::v565)
        writer.write<uint16_t>(this->build);
}
#endif

} // namespace fb::protocol::gateway::request
