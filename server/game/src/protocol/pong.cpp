#include <fb/game/protocol/pong.h>

namespace fb::protocol::game::request {

#ifndef BOT
async::task<void> pong::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->token   = reader.read<uint32_t>();
    this->unknown = reader.read<uint32_t>();
}
#else
async::task<void> pong::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint32_t>(this->token);
    writer.write<uint32_t>(this->unknown);
}
#endif

} // namespace fb::protocol::game::request
