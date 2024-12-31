#include <fb/game/protocol/weather.h>

namespace fb::protocol::game::response {

weather::weather(WEATHER_TYPE value) :
    value(value)
{ }

async::task<void> weather::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->value));
    writer.write<uint8_t>(0x00);
}

} // namespace fb::protocol::game::response
