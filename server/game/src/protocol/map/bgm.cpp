#include <fb/game/protocol/map/bgm.h>

namespace fb::protocol::game::response {

#ifndef BOT
map_bgm::map_bgm(uint16_t bgm, uint8_t volume) :
    bgm(bgm),
    volume(volume)
{ }
#endif

#ifndef BOT
void map_bgm::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(AUDIO::BGM));
    writer.write<uint8_t>(TLV_SIZE_BGM);
    writer.write<uint16_t>(this->bgm);
    writer.write<uint16_t>(this->bgm); // duplicate id consumed by client play call
    writer.write<uint8_t>(this->volume);
}
#else
void map_bgm::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // AUDIO::BGM
    reader.read<uint8_t>(); // TLV_SIZE_BGM
    this->bgm = reader.read<uint16_t>();
    reader.read<uint16_t>(); // bgm duplicate
    this->volume = reader.read<uint8_t>();
}
#endif

#ifndef BOT
map_bgm_stop::map_bgm_stop(uint16_t bgm_id) :
    bgm_id(bgm_id)
{ }

void map_bgm_stop::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(AUDIO::BGM_STOP));
    writer.write<uint8_t>(TLV_SIZE_BGM_STOP);
    writer.write<uint16_t>(this->bgm_id);
}
#else
void map_bgm_stop::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // AUDIO::BGM_STOP
    reader.read<uint8_t>(); // TLV_SIZE_BGM_STOP
    this->bgm_id = reader.read<uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
