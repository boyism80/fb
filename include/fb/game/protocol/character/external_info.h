#ifndef __PROTOCOL_GAME_EXTERNAL_INFO_H__
#define __PROTOCOL_GAME_EXTERNAL_INFO_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class external_info : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x34;

public:
#ifndef BOT
    fb::game::character&    ch;
    const fb::model::model& model;
#else

#endif

public:
#ifndef BOT
    external_info(fb::game::character& ch, const fb::model::model& model);
#else
    external_info() = default;
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