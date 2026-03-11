#ifndef __PROTOCOL_GAME_CONFIG_H__
#define __PROTOCOL_GAME_CONFIG_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/map.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class map_config : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x15;

public:
#ifndef BOT
    const fb::game::map& map;
#else
    uint16_t                  id;
    fb::model::size<uint16_t> size;
    bool                      building;
    std::string               name;
#endif

public:
#ifndef BOT
    map_config(const fb::game::map& map);
#else
    map_config() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif