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
    uint8_t              unknown_extra = 0x00;
    uint16_t             unknown_light = 0;
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
    map_config(const fb::game::map& map);

    map_config(const fb::game::map& map, uint8_t extra, uint16_t light);
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
template <>
void map_config<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#endif

using map_config_v550 = map_config<CLIENT_VERSION::v550>;
using map_config_v565 = map_config<CLIENT_VERSION::v565>;
using map_config_v651 = map_config<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
