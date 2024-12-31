#include <fb/game/protocol/map/map_bgm.h>

namespace fb::protocol::game::response {

#ifndef BOT
map_bgm::map_bgm(const fb::game::map& map, uint8_t volume) :
    map(map),
    volume(volume)
{ }
#endif

#ifndef BOT
async::task<void> map_bgm::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);

    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(0x05);
    writer.write<uint16_t>(this->map.model.id);
    writer.write<uint16_t>(this->map.model.id);
    writer.write<uint8_t>(volume); // volume
    writer.write<uint16_t>(512);
    writer.write<uint16_t>(512);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> map_bgm::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
