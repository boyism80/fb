#include <fb/game/protocol/emotion.h>

namespace fb::protocol::game::request {

#ifndef BOT // server only
async::task<void> emotion::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->value = reader.read<uint8_t>();
}
#else // bot only
emotion::emotion(uint8_t value) :
    value(value)
{ }

async::task<void> emotion::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->value);
}
#endif

} // namespace fb::protocol::game::request
