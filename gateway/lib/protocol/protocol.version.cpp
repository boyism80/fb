#include <fb/gateway/protocol/version.h>

namespace fb::protocol::gateway::request {

#ifdef BOT
version::version(uint16_t v, uint8_t nation) :
    v(v),
    nation(nation)
{ }
#endif

#ifdef BOT
async::task<void> version::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint16_t>(this->v);
    writer.write<uint8_t>(this->nation);
}
#else
async::task<void> version::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->v      = reader.read<uint16_t>();
    this->nation = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::gateway::request
