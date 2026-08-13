#ifndef __PROTOCOL_GAME_CONFIG_H__
#define __PROTOCOL_GAME_CONFIG_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/map.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class map_config : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x15;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const fb::game::map& map;
    // v651-only overrides (defaults safe/unknown; probe builtins may set)
    uint8_t  unknown_flags = 0x02; // bit1 set: avoid +1026==0 hang candidate
    uint8_t  unknown_extra = 0x00; // extra==0 → client +1024=1
    uint16_t unknown_light = 0;
#else
    uint16_t                  id;
    fb::model::size<uint16_t> size;
    bool                      building = false;
    uint8_t                   flags    = 0;
    uint8_t                   extra    = 0;
    uint16_t                  light    = 0;
    std::string               name;
#endif

public:
#ifndef BOT
    map_config(const fb::game::map& map) :
        map(map)
    { }

    map_config(const fb::game::map& map, uint8_t flags, uint8_t extra, uint16_t light) :
        map(map),
        unknown_flags(flags),
        unknown_extra(extra),
        unknown_light(light)
    { }
#else
    map_config() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

#ifndef BOT
template <CLIENT_VERSION V>
void map_config<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    auto building = ENUM_IN(this->map.model().option, MAP_OPTION::BUILD_IN) ? 0x04 : 0x05;
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->map.model().id);
    writer.write<uint16_t>(this->map.width());
    writer.write<uint16_t>(this->map.height());
    writer.write<uint8_t>(building);
    writer.write<std::string, uint16_t>(this->map.model().name);
}

template <>
void map_config<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#endif

using map_config_v550 = map_config<CLIENT_VERSION::v550>;
using map_config_v565 = map_config<CLIENT_VERSION::v565>;
using map_config_v651 = map_config<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
