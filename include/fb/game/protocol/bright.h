#ifndef __PROTOCOL_GAME_BRIGHT_H__
#define __PROTOCOL_GAME_BRIGHT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class bright : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x20;

public:
    const uint8_t value;

public:
    bright(uint8_t value);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif