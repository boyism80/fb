#include <fb/game/protocol/emotion.h>

namespace fb::protocol::game::request {

#ifdef BOT
emotion::emotion(uint8_t value) :
    value(value)
{ }
#endif

#ifdef BOT
async::task<void> emotion::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->value);
}
#else
async::task<void> emotion::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->value = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::request
