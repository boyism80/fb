#ifndef __PROTOCOL_GAME_TIP_H__
#define __PROTOCOL_GAME_TIP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class item_tip : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x59;

public:
    const uint16_t    position;
    const std::string message;

public:
    item_tip(uint16_t position, const std::string& message);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif