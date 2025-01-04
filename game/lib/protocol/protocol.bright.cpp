#include <fb/game/protocol/bright.h>

namespace fb::protocol::game::response {

bright::bright(uint8_t value) :
    value(value)
{ }

async::task<void> bright::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(std::max(0, 20 - this->value));
}
} // namespace fb::protocol::game::response
