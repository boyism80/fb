#include <fb/game/protocol/unknown_35.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> unknown_35::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
async::task<void> unknown_35::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}
#endif

} // namespace fb::protocol::game::response
