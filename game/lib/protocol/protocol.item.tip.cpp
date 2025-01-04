#include <fb/game/protocol/item/item_tip.h>

namespace fb::protocol::game::response {

item_tip::item_tip(uint16_t position, const std::string& message) :
    position(position),
    message(message)
{ }

async::task<void> item_tip::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint16_t>(this->position);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}

} // namespace fb::protocol::game::response
