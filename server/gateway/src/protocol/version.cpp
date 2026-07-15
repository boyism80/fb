#include <fb/gateway/protocol/version.h>

namespace fb::protocol::gateway::request {

#ifndef BOT
void version::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->v      = reader.read<uint16_t>();
    this->nation = reader.read<uint8_t>();
}
#else
version::version(uint16_t v, uint8_t nation) :
    v(v),
    nation(nation)
{ }

void version::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->v);
    writer.write<uint8_t>(this->nation);
}
#endif

} // namespace fb::protocol::gateway::request
