#include <fb/game/protocol/trade/trade_close.h>

namespace fb::protocol::game::response {

#ifndef BOT
trade_close::trade_close(const std::string& message) :
    message(message)
{ }
#endif

#ifndef BOT
async::task<void> trade_close::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x04);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> trade_close::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
