#include <fb/game/protocol/timer.h>

namespace fb::protocol::game::response {

timer::timer(uint32_t time, TIMER_TYPE type) :
    time(time),
    type(type)
{ }

async::task<void> timer::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<uint32_t>(this->time);
    writer.write<uint8_t>(0x00);
}

} // namespace fb::protocol::game::response
