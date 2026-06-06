#include <fb/game/protocol/attack.h>

namespace fb::protocol::game::request {

#ifndef BOT
async::task<void> attack::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}
#else
async::task<void> attack::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#endif

} // namespace fb::protocol::game::request
