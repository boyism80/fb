#include <fb/game/protocol/map/config.h>

namespace fb::protocol::game::response {

#ifndef BOT
map_config::map_config(const fb::game::map& map) :
    map(map)
{ }
#endif

#ifndef BOT
async::task<void> map_config::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    auto building = ENUM_IN(this->map.model.option, MAP_OPTION::BUILD_IN) ? 0x04 : 0x05;
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->map.model.id); // id
    writer.write<uint16_t>(this->map.width());  // width
    writer.write<uint16_t>(this->map.height()); // height
    writer.write<uint8_t>(building);            // this.building ? 0x04 : 0x05
    writer.write<std::string, uint16_t>(this->map.model.name);
}
#else
async::task<void> map_config::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->id          = reader.read<uint16_t>();
    this->size.width  = reader.read<uint16_t>();
    this->size.height = reader.read<uint16_t>();
    this->building    = reader.read<uint8_t>();
    this->name        = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
