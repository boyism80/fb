#include <fb/game/protocol/ad.h>

namespace fb::protocol::game::response {

ad::ad(uint32_t width, uint32_t height, std::string url, uint8_t time) :
    width(width),
    height(height),
    url(url),
    time(time)
{ }

async::task<void> ad::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<std::string, uint16_t>(this->url);
    writer.write<uint16_t>(this->width);
    writer.write<uint16_t>(this->height);
    writer.write<uint8_t>(this->time);
    writer.write<uint8_t>(0x00);
}

} // namespace fb::protocol::game::response
