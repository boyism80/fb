#ifndef __PROTOCOL_GAME_WORLDS_H__
#define __PROTOCOL_GAME_WORLDS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class map_worlds : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x2E;

public:
#ifndef BOT
    const fb::model::model& model;
    const uint32_t          id;
    const uint16_t          index;
#else

#endif

public:
#ifndef BOT
    map_worlds(const fb::model::model& model, uint32_t id, uint16_t index);
#else
    map_worlds() = default;
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