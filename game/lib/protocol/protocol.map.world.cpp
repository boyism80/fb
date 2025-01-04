#include <fb/game/protocol/map/map_world.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> map_world::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    // TODO: serialize bytes
}
#else
async::task<void> map_world::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->value  = reader.read<uint16_t>();
    this->before = reader.read<uint16_t>();
    this->after  = reader.read<uint16_t>();
}
#endif

} // namespace fb::protocol::game::request
