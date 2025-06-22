#ifndef __PROTOCOL_GAME_CLICK_H__
#define __PROTOCOL_GAME_CLICK_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class click : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x43;

public:
    uint32_t fd;

public:
    click() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif