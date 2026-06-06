#include <fb/game/protocol/character/id.h>

namespace fb::protocol::game::response {

#ifndef BOT
id::id(const fb::game::character& ch) :
    ch(ch)
{ }
#endif

#ifndef BOT
async::task<void> id::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->ch.oid());
    writer.write<uint32_t>(static_cast<uint32_t>(this->ch.direction())); // side
    writer.write<uint8_t>(static_cast<uint8_t>(this->ch.cls()));         // class
    writer.write<uint16_t>(0x00);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> id::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->oid       = reader.read<uint32_t>();
    this->direction = reader.read<uint32_t>();
    this->cls       = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
