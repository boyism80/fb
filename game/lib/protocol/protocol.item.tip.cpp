#include <fb/game/protocol/item/item_tip.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> item_tip::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint16_t>(this->position);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> item_tip::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->position = reader.read<uint16_t>();
    this->message  = reader.read<std::string, uint16_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
