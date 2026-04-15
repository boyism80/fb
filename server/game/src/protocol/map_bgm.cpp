#include <fb/game/protocol/map/map_bgm.h>

namespace fb::protocol::game::response {

#ifndef BOT
map_bgm::map_bgm(uint16_t bgm, uint8_t volume) :
    bgm(bgm),
    volume(volume)
{ }
#endif

#ifndef BOT
async::task<void> map_bgm::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);

    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(0x05);
    writer.write<uint16_t>(this->bgm);
    writer.write<uint16_t>(this->bgm);
    writer.write<uint8_t>(this->volume);
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
    reader.read<uint8_t>(); // 0x01
    reader.read<uint8_t>(); // 0x05
    this->bgm = reader.read<uint16_t>();
    reader.read<uint16_t>(); // bgm (duplicate)
    this->volume = reader.read<uint8_t>();
    reader.read<uint16_t>(); // 512
    reader.read<uint16_t>(); // 512
    reader.read<uint8_t>();  // 0x00
    reader.read<uint8_t>();  // 0x00
    reader.read<uint8_t>();  // 0x00
}
#endif

} // namespace fb::protocol::game::response
