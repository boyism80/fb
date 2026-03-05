#include <fb/game/protocol/unknown_1B.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> unknown_1B::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
}
#else
async::task<void> unknown_1B::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}
#endif

} // namespace fb::protocol::game::response
