#include <fb/game/protocol/item/item_throws.h>

namespace fb::protocol::game::request {
async::task<void> item_throws::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->all   = reader.read<uint8_t>();
    this->index = reader.read<uint8_t>() - 1;
}
} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
item_throws::item_throws(const fb::game::character& ch, const fb::game::item& item, const point<uint16_t>& to) :
    ch(ch),
    item(item),
    to(to)
{ }
#endif

#ifndef BOT
async::task<void> item_throws::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->ch.sequence());
    writer.write<uint16_t>(this->item.look());
    writer.write<uint8_t>(this->item.color());
    writer.write<uint32_t>(this->item.sequence());
    writer.write<uint16_t>(this->ch.x());
    writer.write<uint16_t>(this->ch.y());
    writer.write<uint16_t>(this->to.x);
    writer.write<uint16_t>(this->to.y);
    writer.write<uint32_t>(0x00000000);
    writer.write<uint8_t>(0x02);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> item_throws::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response