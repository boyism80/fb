#include <fb/game/protocol/trade/trade_lock.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> trade_lock::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x05);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> trade_lock::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x05
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
