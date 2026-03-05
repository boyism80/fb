#include <fb/game/protocol/unknown_6A.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> unknown_6A::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->value);
}
#else
async::task<void> unknown_6A::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->value = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
