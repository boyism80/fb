#include <fb/game/protocol/object/effect.h>

namespace fb::protocol::game::response {

#ifndef BOT
effect::effect(const fb::game::object& me, uint8_t value) :
    me(me),
    value(value)
{ }
#endif

#ifndef BOT
async::task<void> effect::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->me.sequence());
    writer.write<uint8_t>(this->value);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> effect::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
