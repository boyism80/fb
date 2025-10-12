#include <fb/game/protocol/group.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> group::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<std::string, uint8_t>(this->name);
}
#else
async::task<void> group::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->name = reader.read<std::string, uint8_t>();
}
#endif
} // namespace fb::protocol::game::request
