#include <fb/game/protocol/object/hide.h>

namespace fb::protocol::game::response {

#ifndef BOT
hide::hide(const fb::game::object& object) :
    hide(object.oid())
{ }
hide::hide(uint32_t oid) :
    oid(oid)
{ }
#endif

#ifndef BOT
async::task<void> hide::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> hide::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->oid = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
