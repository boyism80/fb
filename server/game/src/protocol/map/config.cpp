#include <fb/game/protocol/map/config.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <>
void map_config<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->map.model().id);
    writer.write<uint16_t>(this->map.width());
    writer.write<uint16_t>(this->map.height());
    writer.write<uint8_t>(this->unknown_flags);
    writer.write<uint8_t>(this->unknown_extra);
    writer.write<std::string, uint8_t>(this->map.model().name);
    writer.write<uint16_t>(this->unknown_light);
}
#else
template <CLIENT_VERSION V>
void map_config<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->id          = reader.read<uint16_t>();
    this->size.width  = reader.read<uint16_t>();
    this->size.height = reader.read<uint16_t>();
    if constexpr (V == CLIENT_VERSION::v651)
    {
        this->flags    = reader.read<uint8_t>();
        this->extra    = reader.read<uint8_t>();
        this->name     = reader.read<std::string, uint8_t>();
        this->light    = reader.read<uint16_t>();
        this->building = this->flags != 0;
    }
    else
    {
        this->flags    = reader.read<uint8_t>();
        this->building = this->flags != 0;
        this->extra    = 0;
        this->name     = reader.read<std::string, uint16_t>();
        this->light    = 0;
    }
}

template void map_config<CLIENT_VERSION::v550>::deserialize(fb::stream_reader<big_endian>&);
template void map_config<CLIENT_VERSION::v565>::deserialize(fb::stream_reader<big_endian>&);
template void map_config<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>&);
#endif

// Explicit instantiation for primary serialize (non-v651)
#ifndef BOT
template void map_config<CLIENT_VERSION::v550>::serialize(fb::stream_writer<big_endian>&) const;
template void map_config<CLIENT_VERSION::v565>::serialize(fb::stream_writer<big_endian>&) const;
#endif

} // namespace fb::protocol::game::response
