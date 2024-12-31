#include <fb/game/protocol/object/direction.h>

namespace fb::protocol::game::request {

#ifdef BOT
direction::direction(DIRECTION value) :
    value(value)
{ }
#endif

#ifdef BOT
async::task<void> direction::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>((uint8_t)this->value);
}
#else
async::task<void> direction::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->value = DIRECTION(reader.read<uint8_t>());
}
#endif

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
direction::direction(const fb::game::object& object) :
    direction(object.sequence(), object.direction())
{ }
direction::direction(uint32_t sequence, DIRECTION value) :
    sequence(sequence),
    value(value)
{ }
#endif

#ifndef BOT
async::task<void> direction::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->sequence);
    writer.write<uint8_t>(static_cast<uint8_t>(this->value));
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> direction::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->sequence = reader.read<uint32_t>();
    this->value    = (DIRECTION)reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response